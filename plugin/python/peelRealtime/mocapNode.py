import maya.cmds as m


'''

#m.loadPlugin('C:/cpp/git/github/mayaMocap/BUILDS/mayaMotive/MotiveRealtime2016.mll')

from peelRealtime import mocapNode
import maya.cmds as m
m.loadPlugin('C:/cpp/git/github/mayaMocap/mayaMotive/x64/Debug2016/MotiveRealtime2016d.mll')

m.file(f=True, new=True)
reload(mocapNode)

x = mocapNode.MotiveNode('Motive')
m.setAttr( x.node + ".motiveIp", "127.0.0.1", typ='string');
m.setAttr( x.node + ".motiveMode", 1)
m.setAttr( x.node + ".localInterface", 1)
x.setLive(True)
print m.getAttr( x.node + ".info")

x.connectNodes()



m.file(f=True, new=True)
m.unloadPlugin('MotiveRealtime2016d')




'''

def connect( node, dataType ) :

    ''' connect the mocap node to markers, creating them if they do not exist
    @param groupName: the name of the group to parent the locators to (world space)
    @param node: the name of the node sending the data
    @param attribute: the attribute (array) on the node sending the data 
    @param channels: the source channels -> locator mapping
    
    Attribute should be segment, rigidbodies or markers
    Channels should be: [ ( 'MarkerTranslate', 't' ) ]
    '''
    
    if dataType == 'segments' :
        groupName = 'SEGMENTS'
        attribute = 'segments'               
        channels =  [ ( 'segmentTranslate', 't'), ('segmentRotate', 'r') ] 
        nameAttr = 'segmentName'
        
    elif dataType == 'markers' :
        groupName = 'MARKERS'
        attribute = 'markers'
        channels = [ ( 'markerTranslate', 't' ) ]
        nameAttr = 'markerName' 
        
    elif dataType == 'rigidbodies' :
        groupName = 'RIGIDBODIES'
        attribute = 'rigidbodies'               
        channels =  [ ( 'rigidbodyTranslate', 't'), ('rigidbodyRotate', 'r') ] 
        nameAttr = 'rigidbodyName'
    
    else :
        raise ValueError( "Invalid data type: " + str(dataType) )
     

    if not m.objExists( '|' + groupName ) :
        m.group(em=True, name=groupName)

    nattr = node + '.' + attribute
    if not m.objExists( nattr ) : raise RuntimeError("Could not find: " + nattr)

    sz = m.getAttr( nattr, size=True )
    if sz == 0 : return 
    
    for i in  m.getAttr( nattr, mi=True ) :
        name = m.getAttr( "%s[%d].%s" % (nattr, i, nameAttr) )
        if name is None : continue
        print name
        if not m.objExists( '|' + groupName + '|' + name ) :
            loc = m.spaceLocator(name=name)[0]
            m.parent(loc, '|' + groupName)
        else : 
            loc = '|' + groupName + '|' + name
            
        for src, dst in channels :
            namechan = name + '.' + dst
            con = m.listConnections( namechan , d=False, s=True, p=True)
            if con : m.disconnectAttr( con[0], namechan )

        for src, dst in channels :
            m.connectAttr( "%s[%d].%s" % (nattr, i, src), loc + "." + dst)      
    

class MotiveNode(object):

    def __init__(self, name ) :
        if m.objExists( name  ) and m.nodeType( name ) == "peelMotive" : 
            self.node = name
        else : 
            self.node = m.createNode("peelMotive", name=name)
            
        m.setAttr( self.node + ".scale", 10)
        
    def setLive( self, val ) :
        m.setAttr( self.node + ".live", val )
        m.getAttr( self.node + ".segments[0].segmentName")
        m.getAttr( self.node + ".rigidbodies[0].rigidbodyName")        
        m.getAttr( self.node + ".markers[0].markerName")
        
    def connectNodes( self ) :    
        connect( self.node, 'segments' )
        connect( self.node, 'markers' )
        connect( self.node, 'rigidbodies' )


class MocapNode(object) :
    
    def __init__(self, name, port ) :
        if m.objExists( name  ) and m.nodeType( name ) == "peelRealtimeMocap" : 
            self.node = name
        else : 
            self.node = m.createNode("peelRealtimeMocap", name=name)
            
        m.setAttr( self.node + ".port", port )
        m.setAttr( self.node + ".scale", 10)
        
    def setLive( self, val ) :
        m.setAttr( self.node + ".live", val )
        m.getAttr( self.node + ".mocap[0].name")
        
    def connectNodes( self ) :
        
        if not m.objExists('|MOCAP') :
            m.group(em=True, name='MOCAP')


        sz = m.getAttr( self.node + ".mocap", size=True )
        if sz == 0 : return 
        
        for i in  m.getAttr( self.node + ".mocap", mi=True ) :
            name = m.getAttr( self.node + ".mocap[%d].name" % i )
            if name is None : continue
            print name
            if not m.objExists( "|MOCAP|" + name ) :
                loc = m.spaceLocator(name=name)[0]
                m.parent(loc, "|MOCAP")
            else : 
                loc = "|MOCAP|" + name
                
            for attr in [ 't', 'r' ] :
                con = m.listConnections( name + "." + attr , d=False, s=True, p=True)
                if con : m.disconnectAttr( con[0], name + "." + attr )

            m.connectAttr( self.node + ".mocap[%d].outputTranslate" % i, loc + ".t")      
            m.connectAttr( self.node + ".mocap[%d].outputRotate" % i, loc + ".r")      