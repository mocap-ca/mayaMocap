from PySide import QtCore, QtGui
import maya.cmds as m

import localsettings, mocapNode

def show() :
    r = RT()
    r.show()
    return r

def loadPlugin() :
    m.loadPlugin(localsettings.plugin)        


class RT(QtGui.QDialog) :
    
    arduinoNode     = localsettings.arduinoNode
    arduinoIP       = localsettings.arduinoIP
    arduinoPort     = localsettings.arduinoPort
    motivePort      = localsettings.motiveRepeaterPort
    arudinoDataPort = localsettings.arudinoDataPort
    localIP         = localsettings.localIP
    
    
    def __init__(self, parent=None) :
        super(RT, self).__init__( parent )
        
        self.motiveObj      = None
        self.arduinoObj     = None

                
        self.resize( 400, 70 )
        self.setWindowTitle( "Peel Realtime" )
                
        self.layout = QtGui.QVBoxLayout(self)
        self.setLayout(self.layout)
        
        self.buttonLayout = QtGui.QHBoxLayout(self)

        self.buttonArduino = QtGui.QPushButton("Arduino", self)
        self.buttonArduino.clicked.connect(self.arduino)        
        self.buttonLayout.addWidget( self.buttonArduino, 0 )

        self.buttonMotive = QtGui.QPushButton("Motive", self)
        self.buttonMotive.clicked.connect(self.motive)        
        self.buttonLayout.addWidget( self.buttonMotive, 0 )

        self.buttonVcamConnect = QtGui.QPushButton("Connect", self)
        self.buttonVcamConnect.clicked.connect(self.vcamConnect)        
        self.buttonLayout.addWidget( self.buttonVcamConnect, 0 )
        
        self.buttonLayout.addStretch(1)
        
        self.layout.addLayout(self.buttonLayout, 0)
        self.layout.addStretch(1)
                
    def arduino(self) :
    
        self.arduinoObj = mocapNode.MocapNode( "RT_ARDUINO", self.arudinoDataPort )
        self.arduinoObj.setLive(1)
        
        if not m.objExists( "DEV_ARDUINO" ) :
            loc = m.spaceLocator(name="DEV_ARDUINO")[0]
            m.connectAttr( self.arduinoObj.node + ".mocap[0].outputTranslate", loc + ".t")            
            
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

        else :
            loc = "DEV_ARDUINO"
            
        loc = m.select("DEV_ARDUINO")
            
            

        
        msg = "DATA %s %d" % (self.localIP, self.arudinoDataPort )
        print msg
        m.mocapPing(port=self.arduinoPort, ip=self.arduinoIP, msg=msg)
        
        print "GO"
        m.mocapPing(port=self.arduinoPort, ip=self.arduinoIP, msg="GO")
        
        
    def motive( self ) :
    
        self.motiveObj = mocapNode.MocapNode( "RT_MOTIVE", self.motivePort)
        self.motiveObj.setLive(1)
        
        
    def vcamConnect( self ) :
    
        if self.motiveObj is None or not m.objExists( self.motiveObj.node ): 
            print str(self.motiveObj)
            m.error("No Motive node")
            
        self.motiveObj.setLive(1)
        self.motiveObj.connectNodes()
            
        if m.objExists( "|MOCAP|VCAM") :
            
            if not m.objExists( "MOCAP_VCAM_OFFSET") :
                
                oset = m.spaceLocator(name = "MOCAP_VCAM_OFFSET")[0]
                m.parent(oset, "|MOCAP|VCAM")
                m.setAttr( ost + ".t", 0, 0, 0 )
                m.setAttr( ost + ".r", 0, 0, 0 )
            else :
                oset = "MOCAP_VCAM_OFFSET"
                
                
            if not m.objExists("|VCAM") :
                cam = m.camera(name = "VCAM")
                m.parentConstraint( oset, cam[0] )
                
                if m.objExists( "DEV_ARDUINO.ox" ) :
                    m.connectAttr( "DEV_ARDUINO.ox", cam[1] + ".focalLength")
                    
                m.select(cam)
            else  :
                m.select( "|VCAM")
            
    
        
        
    