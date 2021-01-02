

const DEG_TO_RAD = Math.PI/180;

function UpdateControl(){
  if((Params.joypad.connected)&&(Params.joypad.useJoystick)){
    Params.control.input.demand.lift  = Params.joypad.thumbstick.right.trigger;
    Params.control.input.demand.pitch = Params.joypad.thumbstick.right.y;
    Params.control.input.demand.roll  = Params.joypad.thumbstick.right.x;
  }else{
    //use sliders for users without a joypad
    Params.control.input.demand.lift  = Params.sliders.lift;
    Params.control.input.demand.pitch = Params.sliders.pitch;
    Params.control.input.demand.roll  = Params.sliders.roll;
  }

  let thrust      = Params.control.input.demand.lift * MAX_REGISTER_RANGE;
  let pitchTheta  = Params.control.input.demand.pitch;
  let rollTheta   = Params.control.input.demand.roll;

  

  if((Params.mpu6050.detected)&&(Params.control.useFeedback)){

    // PITCH feedback subtraction
    Params.control.error.pitch      =   Params.control.input.demand.pitch - Params.mpu6050.pitch;

    // P term
    Params.control.pid.pitch.p = Params.control.error.pitch * Params.control.pid.kP;
    // I term
    Params.control.pid.pitch.i  +=  Params.control.pid.kI * Params.control.error.pitch;
    if(Params.control.pid.pitch.i>Params.control.pid.accumulatorCap){
      Params.control.pid.pitch.i = Params.control.pid.accumulatorCap;
    }   
    if(Params.control.pid.pitch.i< -Params.control.pid.accumulatorCap){
      Params.control.pid.pitch.i = -Params.control.pid.accumulatorCap;
    }

    // D term
    Params.control.pid.pitch.d = (Params.control.error.pitch - Params.control.pid.pitch.oldError) * Params.control.pid.kD;
    Params.control.pid.pitch.oldError = Params.control.error.pitch;

    // P+I+D
    pitchTheta =  0;
    pitchTheta += Params.control.pid.pitch.p;
    pitchTheta += Params.control.pid.pitch.i;
    pitchTheta += Params.control.pid.pitch.d;


    // ROLL feedback subtraction
    Params.control.error.roll      =   Params.control.input.demand.roll - Params.mpu6050.roll;

    // P term
    Params.control.pid.roll.p = Params.control.error.roll * Params.control.pid.kP;
    // I term
    Params.control.pid.roll.i  +=  Params.control.pid.kI * Params.control.error.roll;
    if(Params.control.pid.roll.i>Params.control.pid.accumulatorCap){
      Params.control.pid.roll.i = Params.control.pid.accumulatorCap;
    }   
    if(Params.control.pid.roll.i< -Params.control.pid.accumulatorCap){
      Params.control.pid.roll.i = -Params.control.pid.accumulatorCap;
    }

    // D term
    Params.control.pid.roll.d = (Params.control.error.roll - Params.control.pid.roll.oldError) * Params.control.pid.kD;
    Params.control.pid.roll.oldError = Params.control.error.roll;

    // P+I+D
    rollTheta =  0;
    rollTheta += Params.control.pid.roll.p;
    rollTheta += Params.control.pid.roll.i;
    rollTheta += Params.control.pid.roll.d;


    Params.control.output.pitch.front = thrust + (thrust * Math.sin(DEG_TO_RAD * pitchTheta));
    Params.control.output.pitch.rear  = thrust - (thrust * Math.sin(DEG_TO_RAD * pitchTheta));
    Params.control.output.roll.left   = thrust + (thrust * Math.sin(DEG_TO_RAD * rollTheta));
    Params.control.output.roll.right  = thrust - (thrust * Math.sin(DEG_TO_RAD * rollTheta));

  }else{

    Params.control.output.pitch.front = thrust + (thrust * Math.sin(DEG_TO_RAD * pitchTheta));
    Params.control.output.pitch.rear  = thrust - (thrust * Math.sin(DEG_TO_RAD * pitchTheta));
    Params.control.output.roll.left   = thrust + (thrust * Math.sin(DEG_TO_RAD * rollTheta));
    Params.control.output.roll.right  = thrust - (thrust * Math.sin(DEG_TO_RAD * rollTheta));

  }

  ///
  ///< set to int
  ///
  Params.control.output.pitch.front   = Math.floor(Params.control.output.pitch.front);
  Params.control.output.pitch.rear    = Math.floor(Params.control.output.pitch.rear );
  Params.control.output.roll.left     = Math.floor(Params.control.output.roll.left  );
  Params.control.output.roll.right    = Math.floor(Params.control.output.roll.right );

  //range capping
  if(Params.control.output.pitch.front > MAX_REGISTER_RANGE){Params.control.output.pitch.front = MAX_REGISTER_RANGE;} 
  if(Params.control.output.pitch.rear  > MAX_REGISTER_RANGE){Params.control.output.pitch.rear  = MAX_REGISTER_RANGE;}
  if(Params.control.output.roll.left   > MAX_REGISTER_RANGE){Params.control.output.roll.left   = MAX_REGISTER_RANGE;} 
  if(Params.control.output.roll.right  > MAX_REGISTER_RANGE){Params.control.output.roll.right  = MAX_REGISTER_RANGE;}

  if(Params.control.output.pitch.front < 0){Params.control.output.pitch.front = 0;} 
  if(Params.control.output.pitch.rear  < 0){Params.control.output.pitch.rear  = 0;}
  if(Params.control.output.roll.left   < 0){Params.control.output.roll.left   = 0;} 
  if(Params.control.output.roll.right  < 0){Params.control.output.roll.right  = 0;}


  PrintControl(thrust);
  OutputControlSignals();
}



  
function OutputControlSignals(){
  //send websockets
}







function PrintUserControls(){
    


    let html =`
    <div>
        <h4>MODE SELECTION</h4>
        Enable Remote Control of Inverters : <input id="CHECK_ENABLE_REMOTE_CONTROL" type="checkbox" value="${Params.control.enableRemoteControl}"><br/>
        Use Closed Loop Feedback : <input id="CHECK_USE_CLOSED_LOOP" type="checkbox" value="${Params.control.useFeedback}"><br/>
        <div>
            <input type="radio" id="RADIO_JOYSTICK" name="CONTROL_TYPE" value="false">
            <label for="JOYSTICK">Use Joystick (XBOX360)</label><br>
            <input type="radio" id="RADIO_SLIDERS" name="CONTROL_TYPE" value="true" checked>
            <label for="SLIDERS">Use Sliders</label><br></br>
        </div>
    </div>    
    `;


    $('#DIV_USER_CONTROLS').html(html);
    $('#CHECK_USE_CLOSED_LOOP').on('change', function() {
        Params.control.useFeedback = this.checked;
        UpdateControl();
    });

    $('#CHECK_ENABLE_REMOTE_CONTROL').on('change', function() {
      Params.control.enableRemoteControl = this.checked;
      UpdateControl();
      EnableDisableRemoteControl();
  });

    $('#RADIO_JOYSTICK').on('change', function() {
        Params.joypad.useJoystick = this.checked;
        UpdateJoypad();
        DrawSliderControls();
    });
    $('#RADIO_SLIDERS').on('change', function() {
        Params.joypad.useJoystick = !this.checked;
        UpdateJoypad();
        DrawSliderControls();
    });


}




function DrawSliderControls(){
    let html='';
    if(!Params.joypad.useJoystick){
        html =
        `
        <div>
            <h4>SLIDERS</h4>
            <p>Pitch Demand (degrees)</p>
            <input id="SLIDER_PITCH_DEM" type="range" min="-45" max="45" value="${Params.sliders.pitch}">${Params.sliders.pitch}&#176;
            <p>Roll Demand (degrees)</p>
            <input id="SLIDER_ROLL_DEM" type="range" min="-45" max="45" value="${Params.sliders.roll}">${Params.sliders.roll}&#176;
            <p>Lift Demand </p>
            <input id="SLIDER_LIFT_DEM" type="range" min="0" max="100" value="${Params.sliders.lift*100}"> ${Params.sliders.lift}
            <br/>
            <button id="BTN_SLIDE_DEFAULTS"> Reset </button>
        </div>
        `; 
    }else{
        html=`<div></div>`;
    }
    $('#DIV_SLIDERS').html(html);

    $('#SLIDER_PITCH_DEM').on('change', function() {
        Params.sliders.pitch = $('#SLIDER_PITCH_DEM').val();
        PrintUserControls();
        DrawSliderControls();
    });
    $('#SLIDER_ROLL_DEM').on('change', function() {
        Params.sliders.roll = $('#SLIDER_ROLL_DEM').val();
        PrintUserControls();
        DrawSliderControls();
    });
    $('#SLIDER_LIFT_DEM').on('change', function() {
        Params.sliders.lift = $('#SLIDER_LIFT_DEM').val()/100;
        PrintUserControls();
        DrawSliderControls();
    });
    $('#BTN_SLIDE_DEFAULTS').on('click', function() {
        Params.sliders.pitch = 0;
        Params.sliders.roll  = 0;
        Params.sliders.lift  = 0;
        PrintUserControls();
        DrawSliderControls();
    });
    UpdateControl();
}





function EnableDisableRemoteControl(){
  if(Params.comms.connected){
    let msg = null;

    msg = {
      sender:       "CONTROLLER",
      destination:  "INVERTER_PITCH",
      msg:          "UPDATE",
      payload:{
        REMOTE_CONTROL_ENABLE:Params.control.enableRemoteControl
      }
    };
    Params.comms.wsHandle.send(JSON.stringify(msg));
  }
}



function PrintControl(thrust){
  let html =`
  </div>
      
      <h4>PID CONTROL</h4>
      Remote Control Enabled:       ${Params.control.enableRemoteControl}<br/>
      Using Closed Loop Feedback :  ${Params.control.useFeedback}<br/>
      Pitch Demand :                ${Params.control.input.demand.pitch}&#176;<br/>
      Roll Demand:                  ${Params.control.input.demand.roll}&#176;<br/>
      Lift Demand:                  ${Params.control.input.demand.lift}<br/>
      Thrust:                       ${thrust}
      <br/>
      Pitch front Output: ${Params.control.output.pitch.front }<br/>
      Pitch rear Output:  ${Params.control.output.pitch.rear  }<br/>
      Roll Left Output:   ${Params.control.output.roll.left   }<br/>
      Roll Right Output:  ${Params.control.output.roll.right  }<br/>
  </div>    
`;
$('#DIV_CONTROL').html(html);
}







//transmit deltas in demand and once per second
var oldMsg1=null;
var oldMsg2=null;
var firstRun = true;
var lastTime = new Date();
function RealTimeControlLoop(){
  UpdateControl();
  if(Params.comms.connected){
    if(Params.control.enableRemoteControl){
      let msg1 = null;
      let msg2 = null;
      let currTime = new Date();
      msg1 = {
        sender:       "CONTROLLER",
        destination:  "INVERTER_PITCH",
        msg:          "UPDATE",
        payload:{
          AXIS0: Params.control.output.pitch.front,
          AXIS1: Params.control.output.pitch.rear,
        }
      };
      

      msg2 = {
        sender:       "CONTROLLER",
        destination:  "INVERTER_ROLL",
        msg:          "UPDATE",
        payload:{
          AXIS0: Params.control.output.roll.left,
          AXIS1: Params.control.output.roll.right,
        }
      };

      if(firstRun){
        firstRun = false;
        oldMsg1 = msg1;
        oldMsg2 = msg2;
      }

      let tx = false;
      if((oldMsg1.payload.AXIS0!=msg1.payload.AXIS0)||
         (oldMsg1.payload.AXIS1!=msg1.payload.AXIS1)){
        tx = true;
      }
      if((oldMsg2.payload.AXIS0!=msg2.payload.AXIS0)||
         (oldMsg2.payload.AXIS1!=msg2.payload.AXIS1)){
        tx = true;
      }

      oldMsg1 = msg1;
      oldMsg2 = msg2;
      let msg2p = JSON.stringify(msg2);
      

      let msg1p = JSON.stringify(msg1);

      if((currTime-lastTime)>=5000){
        lastTime = currTime;
        tx = true;
      }
      if(tx){
        Params.comms.wsHandle.send(msg1p);
        Params.comms.wsHandle.send(msg2p);
        console.log("sending: "+msg1p);
        console.log("sending: "+msg2p);
      }
      

     
    }
  }
} 