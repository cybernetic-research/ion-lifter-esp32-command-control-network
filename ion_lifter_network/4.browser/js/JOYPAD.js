//https://www.javascripture.com/Gamepad


const MAX_MAPPING = 45; //map joypad thumbstick x-y motion to +/-45degrees directly



window.addEventListener('gamepadconnected', (event) => {
  const update = () => {
    let gp=false;
    for (const gamepad of navigator.getGamepads()) {
        
        if (!gamepad) continue;
        gp=gamepad;
        for (const [index, axis] of gamepad.axes.entries()) {

            //assumptions for xbox 360 pad
            
            if(0==index){
                Params.joypad.thumbstick.left.x = axis * MAX_MAPPING;
            }
            if(1==index){
                Params.joypad.thumbstick.left.y = axis * MAX_MAPPING;
            }
            if(2==index){
                Params.joypad.thumbstick.right.x = axis * MAX_MAPPING;
            }
            if(3==index){
                Params.joypad.thumbstick.right.y = axis * MAX_MAPPING;
            }
        }
        for (const [index, button] of gamepad.buttons.entries()) {
            if(0==index){
                Params.joypad.a = button.value;
            }
            if(1==index){
                Params.joypad.b = button.value;
            }
            if(2==index){
                Params.joypad.x = button.value;
            }
            if(3==index){
                Params.joypad.y = button.value;
            }
            if(4==index){
                Params.joypad.thumbstick.left.shoulder = button.value;
            }
            if(5==index){
                Params.joypad.thumbstick.right.shoulder = button.value;
            }
            if(6==index){
                Params.joypad.thumbstick.left.trigger = button.value;
            }
            if(7==index){
                Params.joypad.thumbstick.right.trigger = button.value;
            }
            if(8==index){
                Params.joypad.back = button.value;
            }
            if(9==index){
                Params.joypad.start = button.value;
            }
            if(10==index){
                Params.joypad.thumbstick.left.button = button.value;
            }
            if(11==index){
                Params.joypad.thumbstick.right.button = button.value;
            }
            if(12==index){
                Params.joypad.dpad.up = button.value;
            }
            if(13==index){
                Params.joypad.dpad.down = button.value;
            }
            if(14==index){
                Params.joypad.dpad.left = button.value;
            }
            if(15==index){
                Params.joypad.dpad.right = button.value;
            }
          }
      }
      if(!gp){
          NoJoypad();
      }else{
        Params.joypad.connected = true;
          UpdateJoypad();
          UpdateControl();
      }
   
    requestAnimationFrame(update);
  };
  update();
});






window.addEventListener('gamepaddisconnected', (event) => {
  console.log('gamepaddisconnected:', event.gamepad.connected);
  Params.joypad.connected = false;
  NoJoypad();
});



function NoJoypad(){
  let html =`
  </div>
      
    <h4>JOYPAD</h4>
      Not Detected
  </div>    
`;
$('#DIV_JOYPAD').html(html);
}


function UpdateJoypad(){
    let html =`
    </div>
        
      <h4>JOYPAD</h4>
        Use Joystick: ${Params.joypad.useJoystick }<br/>
        Left stick X : ${Params.joypad.thumbstick.left.x}<br/>
        Left stick Y : ${Params.joypad.thumbstick.left.y}<br/>
        Right stick X: ${Params.joypad.thumbstick.right.x}<br/>
        Right stick Y: ${Params.joypad.thumbstick.right.y}<br/>
        Right Trigger: ${Params.joypad.thumbstick.right.trigger}<br/>
    </div>    
  `;
  if(!Params.joypad.useJoystick){
    html='<div></div>';
  }
  $('#DIV_JOYPAD').html(html);
  }


