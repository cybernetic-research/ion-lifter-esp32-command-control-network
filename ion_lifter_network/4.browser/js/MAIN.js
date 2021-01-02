          
const MAX_REGISTER_RANGE = 255;

  





var Params={
  ///
  ///< comms
  ///
  comms:{
    connected:  false,
    wsHandle:   null,
  },
  ///
  ///< sensor
  ///
  mpu6050:{
    detected: false,
    heading:  0,
    pitch:    0,
    roll:     0,
    error:    false
  },
  ///
  ///< inverter (high voltage ion thrusters)
  ///
  inverter:{
    pitch:{
      detected:       false,
      adc0:           0,
      adc1:           0,
      ctrl0:          0,
      ctrl1:          0,
      remoteControl:  0,
    },
    roll:{
      detected:       false,
      adc0:           0,
      adc1:           0,
      ctrl0:          0,
      ctrl1:          0,
      remoteControl:  0,
    },
  },
  ///
  ///< xbox 360 assumed
  ///
  joypad:{
    useJoystick: false,
    connected: false,
    dpad:{
      up:     false,
      down:   false,
      left:   false,
      right:  false,
    },
    x:        false,
    y:        false,
    a:        false,
    b:        false,
    start:    false,
    back:     false,
    thumbstick:{
      left:{
        x:        0,
        y:        0,
        shoulder: 0,
        trigger:  0,
        button:   false,
      },
      right:{
        x:        0,
        y:        0,
        shoulder: 0,
        trigger:  0,
        button:   false,
      }
    }
    
  },
  sliders:{
    pitch: 0,
    roll:  0,
    lift:  0,
  },
  ///
  ///<
  ///
  control:{
    enableRemoteControl:  false,      // <-- allow user to drive ion thrusters - DANGER 
    useFeedback:          false,
    pid:{
      kP:           1,
      kI:           0,
      kD:           0,
      pitch:{
        i:  0,
        d:   0,
        oldError:     0,
        p:            0,
      },
      roll:{
        i:  0,
        d:   0,
        oldError:     0,
        p:            0,
      },
      accumulatorCap: 255,
    },
    error:{
      pitch:    0,
      roll:     0,
    },
    input:{
      feedback:{
        pitch:  0,
        roll:   0,
      },
      demand:{
        pitch:  0,
        roll:   0,
        lift:   0,
      },
    },
    output:{
      lift:     0,
      roll:{
        left:   0,
        right:  0,
      },
      pitch:{
        front:  0,
        rear:   0,
      },
    }
  }
};




$(document).ready(function(){
  PrintUserControls();
  UpdateJoypad();
  CommsBegin();
  NoJoypad();
  PrintEuler();
  DrawSliderControls();
  PrintInverter();
  setInterval(function(){RealTimeControlLoop()},200);
});

/*
{"sender":"SENSOR_ORIENTATION","msg":"STATUS","payload":{"HEADING":"0.063068","PITCH":"-0.859255","ROLL":"-0.264984"}}
*/
function UpdateEuler(input){
  Params.mpu6050.detected = true;
  Params.mpu6050.heading  = input.payload.HEADING;
  Params.mpu6050.pitch    = input.payload.PITCH;
  Params.mpu6050.roll     = input.payload.ROLL;
  UpdateControl();
  PrintEuler();
}



function PrintEuler(){
	let html =`
        </div>
            
                <h4>MPU6050 EULER ANGLES</h4>
                Detected: ${Params.mpu6050.detected}<br/>
                Heading:  ${Params.mpu6050.heading}&#176;<br/>
                Pitch:    ${Params.mpu6050.pitch}&#176;<br/>
                Roll:     ${Params.mpu6050.roll}&#176;<br/>
            
        </div>    
    `;
    $('#DIV_EULER').html(html);
}






