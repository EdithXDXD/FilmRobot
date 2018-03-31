/******************************************************************************\
 * Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
 * Leap Motion proprietary and confidential. Not for distribution.              *
 * Use subject to the terms of the Leap Motion SDK Agreement available at       *
 * https://developer.leapmotion.com/sdk_agreement, or another agreement         *
 * between Leap Motion and you, your company or other organization.             *
 \******************************************************************************/

#include <iostream>
#include <cstring>
#include <fstream>
#include "Leap.h"

using namespace Leap;

/*
"MotionState": [
             {
             "direction": "left"
             "speed":"50.00"
             }
             ]
 
 */

class SampleListener : public Listener {
public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);
//    web::json::value createJson(std::string direction, float speed);
    void createJson(std::string direction, float speed);
    enum SignificantDirection {
        vertical,horizontal
    };
    
    enum FilmDirection {
        film_stop, film_left, film_right, film_forward, film_backward
    };
    
    
private:
    
    SignificantDirection checkSignificantDirection(const Leap::Vector& direction);
    FilmDirection mCurrDirection;
    Hand mCurrHand;
    
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener::createJson(std::string direction, float speed)
{
    std::ofstream output ("data.txt");
    if (output.is_open())
    {
    //output << "[" << std::endl;
        output << "{" << std::endl;
        output << "     \"direction\": " << "\"" << direction << "\"," << std::endl;
        output << "     \"speed\": " << "\"" << speed << "\"" << std::endl;
        output << "}" << std::endl;
     //   output << "]" << std::endl;
    }
    output.close();
    
    system("bash sendRequest.sh");
}


void SampleListener::onInit(const Controller& controller) {
    mCurrDirection = film_stop;
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
 //   controller.enableGesture(Gesture::TYPE_CIRCLE);
 // controller.enableGesture(Gesture::TYPE_KEY_TAP);
 //  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();

    
    /**
      Some how critical to config listener to get all the hands
     */
    HandList hands = frame.hands();
    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;
        mCurrHand = hand;
      
        // Get the Arm bone
        Arm arm = hand.arm();
        
        // Get fingers
        const FingerList fingers = hand.fingers();
        for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
            const Finger finger = *fl;

            // Get finger bones
            for (int b = 0; b < 4; ++b) {
                Bone::Type boneType = static_cast<Bone::Type>(b);
                Bone bone = finger.bone(boneType);

            }
        }
    }
    
    
    // Get gestures
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];
        
        switch (gesture.type()) {
            // only support swipe currently
            case Gesture::TYPE_SWIPE:
            {
                SwipeGesture swipe = gesture;
                SignificantDirection realDirect = checkSignificantDirection(swipe.direction());

                if (!mCurrHand.isLeft() && stateNames[gesture.state()] == "STATE_START"  && realDirect == horizontal)
                {
                    if (swipe.direction().x > 0 && mCurrDirection != film_right)
                    {
                        std::cout << "right" << std::endl;
                        mCurrDirection = film_right;
                        createJson("right", swipe.speed());
                    }
                    if (swipe.direction().x < 0 && mCurrDirection != film_left)
                    {
                        std::cout << "left" << std::endl;
                        mCurrDirection = film_left;
                        createJson("left", swipe.speed());
                    }
                }
                else if (!mCurrHand.isLeft() && stateNames[gesture.state()] == "STATE_START" && realDirect == vertical)
                {
                    if (swipe.direction().z > 0 && mCurrDirection != film_backward)
                    {
                        mCurrDirection = film_backward;
                        std::cout << "backward" << std::endl;
                        createJson("backward", swipe.speed());
                    }
                    if (swipe.direction().z < 0 && mCurrDirection != film_forward)
                    {
                        mCurrDirection = film_forward;
                        createJson("forward", swipe.speed());
                    }
                }
                
                if (mCurrHand.isLeft() && mCurrDirection != film_stop)
                {
                    mCurrDirection = film_stop;
                    std::cout << "stop" << std::endl;
                    createJson("stop", swipe.speed());
                }
                
                break;
            }
            default:
                std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
                break;
        }
    }
    
    if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
        // std::cout << std::endl;
    }
    
}

SampleListener::SignificantDirection SampleListener::checkSignificantDirection(const Leap::Vector& direction)
{
    float absoluteX = std::abs(direction.x);
    float absoluteZ = std::abs(direction.z);
    return absoluteX < absoluteZ ? vertical:horizontal;
   
}
void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
    std::cout << "Device Changed" << std::endl;
    const DeviceList devices = controller.devices();
    
    for (int i = 0; i < devices.count(); ++i) {
        std::cout << "id: " << devices[i].toString() << std::endl;
        std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    }
}

void SampleListener::onServiceConnect(const Controller& controller) {
    std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
    std::cout << "Service Disconnected" << std::endl;
}


//web::json::value SampleListener::createJson(std::string direction, float speed){
//    web::json::value retVal;
////    web::json::value motionState;
////    motionState["direction"] = web::json::value::string(direction);
////    motionState["speed"]  = web::json::value::number(speed);
////    std::vector<web::json::value> arrayStates;
////    arrayStates.push_back(motionState);
////    retVal["MotionState"] = web::json::value::array(arrayStates);
//    return retVal;
//}

int main(int argc, char** argv) {
    
    // Create a sample listener and controller
    SampleListener listener;
    Controller controller;

    // Have the sample listener receive events from the controller
    controller.addListener(listener);

    if (argc > 1 && strcmp(argv[1], "--bg") == 0)
        controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

    // Keep this process running until Enter is pressed
     std::cout << "Press Enter to quit..." << std::endl;
     std::cin.get();

    // Remove the sample listener when done
     controller.removeListener(listener);
    
    return 0;
}

