
/*
{"sender":"SENSOR_ORIENTATION","msg":"STATUS","payload":{"HEADING":"0.063068","PITCH":"-0.859255","ROLL":"-0.264984"}}
*/
function UpdateInverter(input){
    if(input.sender=="INVERTER_PITCH"){
        Params.inverter.pitch.detected      = true;
        Params.inverter.pitch.adc0          = input.payload.ADC0;
        Params.inverter.pitch.adc1          = input.payload.ADC1;
        Params.inverter.pitch.ctrl0         = input.payload.CTRL0;
        Params.inverter.pitch.ctrl1         = input.payload.CTRL1;
        Params.inverter.pitch.remoteControl = input.payload.REMOTE_CONTROL;
    }else if(input.sender=="INVERTER_ROLL"){
        Params.inverter.roll.detected      = true;
        Params.inverter.roll.adc0          = input.payload.ADC0;
        Params.inverter.roll.adc1          = input.payload.ADC1;
        Params.inverter.roll.ctrl0         = input.payload.CTRL0;
        Params.inverter.roll.ctrl1         = input.payload.CTRL1;
        Params.inverter.roll.remoteControl = input.payload.REMOTE_CONTROL;
    }
    PrintInverter();
  }
  
  
  
  function PrintInverter(){
      let html =`
          </div>
            
            <h4>INVERTER PITCH</h4>
            Detected: ${Params.inverter.pitch.detected}<br/>
            ADC0:     ${Params.inverter.pitch.adc0}<br/>
            ADC1:     ${Params.inverter.pitch.adc1}<br/>
            CTRL0:    ${Params.inverter.pitch.ctrl0}<br/>
            CTRL1:    ${Params.inverter.pitch.ctrl1}<br/>
            REMOTE CONTROL:     ${Params.inverter.pitch.remoteControl}<br/>

            <h4>INVERTER ROLL</h4>
            Detected: ${Params.inverter.roll.detected}<br/>
            ADC0:     ${Params.inverter.roll.adc0}<br/>
            ADC1:     ${Params.inverter.roll.adc1}<br/>
            CTRL0:    ${Params.inverter.roll.ctrl0}<br/>
            CTRL1:    ${Params.inverter.roll.ctrl1}<br/>
            REMOTE CONTROL:     ${Params.inverter.roll.remoteControl}<br/>
          </div>    
      `;
      $('#DIV_INVERTER').html(html);
  }
  