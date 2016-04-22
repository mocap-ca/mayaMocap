import maya.cmds as m
import maya.mel as mel

    


def jointList() :

    return [
        ( 'Hips',       'RightUpLeg' ),
        ( 'RightUpLeg', 'RightLeg' ),
        ( 'RightLeg',   'RightFoot' ),
        ( 'RightFoot',  'RightToeBase' ),

        ( 'Hips',     'LeftUpLeg' ),
        ( 'LeftUpLeg', 'LeftLeg' ),
        ( 'LeftLeg',   'LeftFoot' ),
        ( 'LeftFoot',  'LeftToeBase' ),
        
        ( 'Hips',   'Spine' ),
        ( 'Spine',  'Spine1' ),
        ( 'Spine1', 'Neck' ),
        ( 'Neck',   'Head' ),
        
        ( 'Spine1',       'LeftShoulder' ),
        ( 'LeftShoulder', 'LeftArm' ),
        ( 'LeftArm',      'LeftForeArm' ),
        ( 'LeftForeArm',  'LeftHand' ),
        
        ( 'Spine1',        'RightShoulder' ),
        ( 'RightShoulder', 'RightArm' ),
        ( 'RightArm',      'RightForeArm' ),
        ( 'RightForeArm',  'RightHand' ) ]
        
        

        
def joints() :
    s = set()
    for i in jointList() : s.update( i )
    return list(s)
        

def create( prefix, namespace, character = None ) :

    ''' create a skeleton for the marker prefix using the specified namespace '''

    m.select(cl=True)

    m.namespace( set=':' )
    m.namespace( add=namespace )
    m.namespace( set=namespace )

    j = m.joint(name='Hips')

    for first, second in jointList() :
        m.select( namespace + ':' + first )
        j = m.joint(name=second)

        dm = m.createNode("decomposeMatrix")
        mm = m.createNode("multMatrix")
        m.connectAttr( prefix + second + ".worldMatrix", mm + ".i[0]" )
        m.connectAttr( prefix + first + ".worldInverseMatrix", mm + ".i[1]")
        m.connectAttr( mm + ".o", dm + ".inputMatrix" )

        tr = m.getAttr( dm + ".ot")[0]

        try :
            m.setAttr( j + ".t", *tr )
            m.connectAttr( dm + ".or", j + ".r" )
        except RuntimeError as e :
            m.warning(str(e))

    try :
        m.connectAttr( prefix + "Hips.t", namespace + ":Hips.t")
    except RuntimeError as e :
        m.warning(str(e))

    try :
        m.connectAttr( prefix + "Hips.r", namespace + ":Hips.r")
    except RuntimeError as e :
        m.warning(str(e))

    
    if character :
        hold(namespace)
        zero(namespace)
        hik(character, namespace)
        unhold(namespace)



def holdAttr( node, attr, holdattr ) :

    ''' move the connection over to a temp attribute '''

    if not m.objExists ( node + "." + attr ) : return
    con = m.listConnections( node + '.' + attr, p=True, s=True, d=False, scn=True ) 
    if not con : return
    
    if not m.objExists( node + '.' + holdattr ) :
        m.addAttr( node, ln=holdattr, k=True, at='double3' )
        m.addAttr( node, ln=holdattr + 'X', k=True, at='double' , parent=holdattr)
        m.addAttr( node, ln=holdattr + 'Y', k=True, at='double' , parent=holdattr)
        m.addAttr( node, ln=holdattr + 'Z', k=True, at='double' , parent=holdattr)
        
    m.connectAttr( con[0], node + '.' + holdattr )
    m.disconnectAttr( con[0], node + '.' + attr )   
    
def unholdAttr( node, attr, holdattr ) :    

    ''' move the connection off the temp attribute back to the normal one '''

    if not m.objExists( node + '.' + holdattr ) : return

    con = m.listConnections( node + '.' + holdattr, p=True, s=True, d=False, scn=True ) 
    if not con : return
        
    m.connectAttr( con[0], node + '.' + attr )
    m.disconnectAttr( con[0], node + '.' + holdattr )   


    
def hold( namespace ) :

    ''' move all the translation/rotation connections on to a temp attribute for the namespace '''

    for joint in joints() :
        holdAttr ( namespace + ':' + joint, 'r', 'holdr' )
        holdAttr ( namespace + ':' + joint, 't', 'holdt' )    
        
def unhold( namespace ) :
    for joint in joints() :
        unholdAttr ( namespace + ':' + joint, 'r', 'holdr' )
        unholdAttr ( namespace + ':' + joint, 't', 'holdt' )    
        
        
def zero( namespace ) :
    items = set()
    for i, j in jointList() :
        for k in [i,j] :
            m.setAttr( namespace + ':' + k + '.r', 0, 0, 0 )
    


def hik( charName, namespace) :

    joints = {}

    for i in  range( mel.eval('hikGetNodeCount()') ) :
        j = namespace + ':' + mel.eval('GetHIKNodeName(%d)' % i )
        if not m.objExists( j ) : continue
        joints[i] = j
        
        charAttr = j + ".Character"
        if m.objExists( charAttr ) :
            con = m.listConnections ( charAttr )
            if con is not None and len( con ) > 0 :
                    m.error( "Already characterized: " + j )        
        
    if len(joints) == 0 :
        m.error( "No joints found")
        return
    
    char = mel.eval('hikCreateCharacter("%s")' % charName)

    for i in joints :
        mel.eval('hikAddSkToCharacter("%s", "%s", %d, false)' % ( char, joints[i], i ) )
        mel.eval('hikReadStancePoseTRSOffsetsForNode("%s", %d)' % ( char, i ) )
        
        
        
def charAll() :

    for i in m.ls( "*:Hips") :
        sp = i.split(':')
        hold( sp[0] )
        zero( sp[0] )
        hik( sp[0], sp[0] )
        unhold( sp[0] )

       
