/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


#include "OscProxyEventHandler.hpp"

static const unsigned long BUFFER_SIZE = 2048;

OscProxyEventHandler::OscProxyEventHandler(const std::string& address, int port)
    : osgGA::GUIEventHandler()
    , _transmitSocket(IpEndpointName(address.c_str(), port))
    , _buffer(new  char[BUFFER_SIZE])
    , _oscStream(_buffer, BUFFER_SIZE)
    , _firstRun(true)
{
    OSG_INFO << "OscDevice :: sending events to " << address << ":" << port << std::endl;
}


OscProxyEventHandler::~OscProxyEventHandler()
{
    delete[] (_buffer);
}

 bool OscProxyEventHandler::handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
{
    bool do_send(false);
    switch(ea.getEventType())
    {
        case osgGA::GUIEventAdapter::FRAME:
            if (_firstRun)
            {
                _firstRun = false;
                sendInit(ea);
                do_send = true;
            }
            break;
            
        case osgGA::GUIEventAdapter::RESIZE:
            sendInit(ea);
            do_send = true;
            
            break;
        case osgGA::GUIEventAdapter::PUSH:
            _oscStream << osc::BeginMessage("/osgga/mouse/press") << ea.getX() << ea.getY() << ea.getButton() << osc::EndMessage;
            do_send = true;
            break;
            
        case osgGA::GUIEventAdapter::RELEASE:
            _oscStream << osc::BeginMessage("/osgga/mouse/release") << ea.getX() << ea.getY() << ea.getButton() << osc::EndMessage;
            do_send = true;
            break;
        
        case osgGA::GUIEventAdapter::DOUBLECLICK:
            _oscStream << osc::BeginMessage("/osgga/mouse/doublepress") << ea.getX() << ea.getY() << ea.getButton() << osc::EndMessage;
            do_send = true;
            break;
            
        case osgGA::GUIEventAdapter::MOVE:
        case osgGA::GUIEventAdapter::DRAG:
            _oscStream << osc::BeginMessage("/osgga/mouse/motion") << ea.getX() << ea.getY() << osc::EndMessage;
            do_send = true;
            break;
        
        case osgGA::GUIEventAdapter::KEYDOWN:
            _oscStream << osc::BeginMessage("/osgga/key/press") << ea.getKey() << osc::EndMessage;
            do_send = true;
            break;
        
        case osgGA::GUIEventAdapter::KEYUP:
            _oscStream << osc::BeginMessage("/osgga/key/release") << ea.getKey() << osc::EndMessage;
            do_send = true;
            break;
        
        default:
            break;
        
    }
    if (do_send)
    {
        OSG_INFO << "OscDevice :: sending event per OSC " << std::endl;
        
        _transmitSocket.Send( _oscStream.Data(), _oscStream.Size() );
        _oscStream.Clear();
    }
    
    return false;
}

void OscProxyEventHandler::sendInit(const osgGA::GUIEventAdapter &ea)
{
    _oscStream << osc::BeginBundle();
    _oscStream << osc::BeginMessage("/osgga/resize") << ea.getWindowX() << ea.getWindowY() << ea.getWindowWidth() << ea.getWindowHeight() << osc::EndMessage;
    _oscStream << osc::BeginMessage("/osgga/mouse/set_input_range") << ea.getXmin() << ea.getYmin() << ea.getXmax() << ea.getYmax() << osc::EndMessage;
    _oscStream << osc::BeginMessage("/osgga/mouse/y_orientation_increasing_upwards") << (bool)(ea.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS)  << osc::EndMessage;
    _oscStream << osc::EndBundle;
}
