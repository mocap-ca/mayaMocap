'''
import maya.cmds as m
from peelRealtime import gui

#m.loadPlugin('C:/cpp/git/github/mayaMocap/mayaMotive/x64/Debug2016/MotiveRealtime2016d.mll')

m.loadPlugin('C:/cpp/git/github/mayaMocap/mayaMotive/x64/R2016.5/MotiveRealtime2016.mll')

m.loadPlugin('C:/cpp/git/github/mayaMocap/plugin/build_vs2015/x64/Release/build_vs2015.mll')


gui.show()

'''





from PySide import QtCore, QtGui

import maya.cmds as m
import maya.OpenMayaUI as omui
import shiboken

import localsettings, mocapNode

INSTANCE = None

def show() :

    global INSTANCE
    
    INSTANCE = RT()
    INSTANCE.show()
    return INSTANCE

def loadPlugin() :
    m.loadPlugin(localsettings.plugin)        


class RT(QtGui.QDialog) :
    
    arduinoNode     = localsettings.arduinoNode
    arduinoIP       = localsettings.arduinoIP
    arduinoPort     = localsettings.arduinoPort
    motivePort      = localsettings.motiveRepeaterPort
    arudinoDataPort = localsettings.arudinoDataPort
    repeaterPort    = localsettings.motiveRepeaterPort
    localIP         = localsettings.localIP
    
    
    def __init__(self, parent=None) :
    
        if parent is None : 
            ptr = omui.MQtUtil.mainWindow()
            parent = shiboken.wrapInstance(long(ptr), QtGui.QWidget)
            
        super(RT, self).__init__( parent )
        
                
        self.resize( 400, 70 )
        self.setWindowTitle( "Peel Realtime" )
                
        self.layout = QtGui.QVBoxLayout(self)
        self.setLayout(self.layout)
        
        self.items = QtGui.QListWidget(self)
        self.items.itemClicked.connect(self.listItemClicked)
        self.layout.addWidget(self.items)
        
        self.log = QtGui.QTextEdit(self)
        self.layout.addWidget(self.log)        
        
        self.lowbar = QtGui.QHBoxLayout()
        self.layout.addLayout(self.lowbar, 0)
        
        self.cbLive = QtGui.QCheckBox("Live", self)
        self.lowbar.addWidget(self.cbLive)
        self.cbLive.stateChanged.connect(self.setLive)
        
        self.reloadButton = QtGui.QPushButton("Reload", self)
        self.reloadButton.clicked.connect( self.populate )
        self.lowbar.addWidget(self.reloadButton)
        
        # Menu
               
        self.menu = QtGui.QMenuBar()
        self.connectMenu = QtGui.QMenu("Connect")
        self.menu.addMenu(self.connectMenu)        
        
        arduinoAction = self.connectMenu.addAction( "Arduino" )
        arduinoAction.triggered.connect( self.createArduino )

        motiveAction = self.connectMenu.addAction( "Motive" )
        motiveAction.triggered.connect( self.createMotiveNode )

        repeaterAction = self.connectMenu.addAction( "Repeater" )
        repeaterAction.triggered.connect( self.createUdpNode )
        
        self.rigMenu = QtGui.QMenu("Rig")
        self.menu.addMenu(self.rigMenu)
        
        rigRtAction = self.rigMenu.addAction( "Mocap" )
        rigRtAction.triggered.connect( self.rtConnect )

        rigVcamAction = self.rigMenu.addAction( "VCAM" )
        rigVcamAction.triggered.connect( self.vcamConnect )
        
        
        self.layout.setMenuBar( self.menu) 
        
        self.populate()

    def populate(self) :
    
        ''' populate the gui using data in the scene '''
    
        self.items.clear()
        
        state = set()
        
        for i in m.ls(typ='peelMotive') :
            lwi =  QtGui.QListWidgetItem( str(i) + "  (Motive)" )
            lwi.setData( QtCore.Qt.UserRole, str(i) )
            self.items.addItem( lwi )
            state.add( m.getAttr( i + ".live" ) )
            
        for i in m.ls(typ='peelRealtimeMocap') :
            lwi =  QtGui.QListWidgetItem( str(i) + "  (UDP)" )
            lwi.setData( QtCore.Qt.UserRole, str(i) )
            self.items.addItem( lwi )
            state.add( m.getAttr( i + ".live") )
            
            
        
        if len(state) == 0 :
            self.cbLive.setCheckState( QtCore.Qt.Unchecked )
            
        elif len(state) == 1 and 0 in state :
            self.cbLive.setCheckState( QtCore.Qt.Unchecked )
        
        elif len(state) == 1 and 1 in state :
            self.cbLive.setCheckState( QtCore.Qt.Unchecked )
            
        elif len(state) > 1 : 
            self.cbLive.setCheckState( QtCore.Qt.PartiallyChecked )
            
        else :
            print "Could not determine live state: " + str( state ) 


    def logMessage( self, value ) :
        self.log.append(value)
        
    def listItemClicked(self, item) :
        try :
            node = item.data(QtCore.Qt.UserRole) 
            m.select( node )
        except Exception as e :
            m.warning(str(e))


    ####################
    # CREATE
                
    def createArduino(self) :
    
        ''' Create the arduino nodes '''
        
        if m.objExists( 'RT_ARDUINO' ) :
            self.logMessage( 'node already exists: RT_ARDUINO' )
            return
        
        self.logMessage("Connecting to arduino on port: %d" % self.arudinoDataPort )
    
        arduinoObj = mocapNode.UdpNode( "RT_ARDUINO", self.arudinoDataPort )
        
        self.logMessage( "Created arduino node as: " + arduinoObj.node )

        
        loc = m.spaceLocator(name="DEV_ARDUINO")[0]
        m.connectAttr( arduinoObj.node + ".mocap[0].outputTranslate", loc + ".t")            
        
        m.addAttr( loc, sn='prx', ln='preX', k=True)
        m.addAttr( loc, sn='pry', ln='preY', k=True)
        m.addAttr( loc, sn='mx', ln='multX', k=True)
        m.addAttr( loc, sn='my', ln='multY', k=True)
        m.addAttr( loc, sn='psx', ln='postX', k=True)
        m.addAttr( loc, sn='psy', ln='postY', k=True)
        m.addAttr( loc, sn='ox', ln='outx', k=True)
        m.addAttr( loc, sn='oy', ln='outy', k=True)            

        pre = m.createNode('plusMinusAverage')            
        mult = m.createNode('multiplyDivide')
        post = m.createNode('plusMinusAverage')            

        # loc -> pre
        m.connectAttr( loc + ".t", pre + ".input3D[0]")
        m.connectAttr( loc + ".preX", pre + ".input3D[1].input3Dx")
        m.connectAttr( loc + ".preY", pre + ".input3D[1].input3Dy")
        
        # pre -> mult             
        m.connectAttr( pre + ".o3", mult + ".input1")

        # mult -> post
        m.connectAttr(mult + ".o", post + ".input3D[0]")
        
        # loc -> mult            
        m.setAttr( loc + ".mx", 1)
        m.setAttr( loc + ".my", 1)
        m.connectAttr( loc + ".mx", mult + ".input2X")
        m.connectAttr( loc + ".my", mult + ".input2Y")
        
        # loc -> post
        m.connectAttr( loc + ".psx", post + ".input3D[1].input3Dx" )
        m.connectAttr( loc + ".psy", post + ".input3D[1].input3Dy" )
        m.setAttr( loc + ".psx", 35)
        
        # post -> loc
        m.connectAttr( post + ".o3x", loc + ".ox")            
        m.connectAttr( post + ".o3y", loc + ".oy")

        m.select("DEV_ARDUINO")
            
        
        #msg = "DATA %s %d" % (self.localIP, self.arudinoDataPort )
        #print msg
        #m.mocapPing(port=self.arduinoPort, ip=self.arduinoIP, msg=msg)
        
        #print "GO"
        #m.mocapPing(port=self.arduinoPort, ip=self.arduinoIP, msg="GO")
        
        
        self.populate()
        

        
    def createMotiveNode( self ) :   
        
        motiveObj = mocapNode.MotiveNode( "RT_MOTIVE")
        m.setAttr( motiveObj.node + ".motiveIp", "127.0.0.1", typ='string');
        m.setAttr( motiveObj.node + ".motiveMode", 1)
        m.setAttr( motiveObj.node + ".localInterface", 1)
        
        self.logMessage("Created motive node as: " + motiveObj.node )
        
        self.populate()
        
    def createUdpNode( self ) :
       
        repeaterObj = mocapNode.UdpNode("RT_REPEATER", self.repeaterPort)
    
        self.logMessage('Created udp node as: ' + repeaterObj.node )
        
        self.populate()
        
        
    ######################
    # Connect    

    def setLive( self, value) :
        print "Set Live: " + str(value)    
        
        if value > 1 : value = 1

        for i in m.ls(typ='peelMotive')  + m.ls(typ='peelRealtimeMocap')  :
            print "Setting " + str(i) + ".node to " + str( value )
            try : 
                m.setAttr( i + ".live", int(value) )
            except Exception as e :
                m.warning( str(e) )
                
                
    def rtConnect( self ) :
    
        ''' Search the scene for realtime nodes and make connections to new locators if needed '''
    
        for motiveNode in m.ls(type='peelMotive') :
            self.logMessage("Connecting motive node: " + str(motiveNode) )
            motiveObj = mocapNode.MotiveNode( motiveNode )
            motiveObj.connectNodes()
            
        for udpNode in m.ls(type='peelRealtimeMocap') :
            self.logMessage("Connecting udp node: " + str(motiveNode) )
            udpObj = mocapNode.UdpNode( udpNode )
            udpObj.connectNodes()
            
    def vcamConnect( self ) :
    
        ''' Connect the MOCAP|VCAM node to a camera '''
        
        found = None
        for node in [ 'VCAM', 'vcam', 'Vcam', 'VCam' ] :
            if m.objExists( "|RIGIDBODIES|" + node) :
                found = node
                break
                
        if not found :
            m.error("Could not find a MOCAP|VCAM node")
            return
                
                
            
        if not m.objExists( "MOCAP_VCAM_OFFSET") :
            
            oset = m.spaceLocator(name = "MOCAP_VCAM_OFFSET")[0]
            m.parent(oset, found )
            m.setAttr( oset + ".t", 0, 0, 0 )
            m.setAttr( oset + ".r", 0, 0, 0 )
        else :
            oset = "MOCAP_VCAM_OFFSET"
            
            
        if not m.objExists("|VCamera") :
            cam = m.camera(name = "VCamera")
            m.parentConstraint( oset, cam[0] )
            
            if m.objExists( "DEV_ARDUINO.ox" ) :
                m.connectAttr( "DEV_ARDUINO.ox", cam[1] + ".focalLength")
                
            m.select(cam)
        else  :
            m.select( "|VCAM")
            
    
        
        
    