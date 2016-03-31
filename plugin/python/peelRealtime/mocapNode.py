import maya.cmds as m

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