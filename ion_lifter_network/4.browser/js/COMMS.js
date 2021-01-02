

var host=0;
var ws 	=0;
host	=	location.origin.replace(/^http/, 'ws');
let host_alt = "ws://192.168.4.1:81";
ws 	= 	new WebSocket(host_alt)
var Comms ={
	lastMessage: 		0,
	receivedMessages: 	0,
	watchdog:{
		enable: 							false,
		lastKick: 							0,
		TIMEOUT_SECONDS:					15,
		SECONDS_BETWEEN_WATCHDOG_CHECKS:	15
	},
};


function CommsBegin(){


	
	
}


ws.onopen = function(event){
	console.log('connected to websocket');
	Comms.lastMessage 		=	0;
	Comms.receivedMessages 	=  	0;
	Params.comms.connected  = true;
	Params.comms.wsHandle	=	ws;
	msg = {
		sender:"CONTROLLER",
		destination:"LIFTER_SERVER",
		msg:"CONNECTED"
	};
	ws.send(JSON.stringify(msg));
}

ws.ondisconnect = function(event){
	console.log('disconnected from websocket');
	//retry
	Params.comms.connected  = true;
}




ws.onmessage = function (event) {
	console.log(event.data);
	
	try{
		let str 	= 	event.data;
		let data 	=	JSON.parse(str);
		let sendMsg	= 	false;
		let msg 	=	null;
		//WatchDogTimerOperations();

		switch(data.msg){


			case "CONNECTED":{
				
			}break;
			
			
			default:{
				msg={sender:"CONTROLLER",
					detination:"ALL"
					};
			}break;


			/*
			{"sender":"SENSOR_ORIENTATION","msg":"STATUS","payload":{"HEADING":"0.063068","PITCH":"-0.859255","ROLL":"-0.264984"}}
			*/
			case "STATUS":{
				switch(data.sender){
					default:break;
					case "SENSOR_ORIENTATION":{
						UpdateEuler(data);
					}break;

					case "INVERTER_ROLL":
					case "INVERTER_PITCH":{
						UpdateInverter(data);
					}break;
				}
			}break;

		};
	

		if(ws !== undefined){
			if(sendMsg){
				ws.send(JSON.stringify(msg));
			}
		}else{
			console.log('error setting up socket');
			//ResetSystem();
		}
	}catch(err){
		console.log("WS Error: "+err);
		console.log(event.data);
	}
}



function SendMessage(msg){
	if(ws !== undefined){
		try{
			ws.send(JSON.stringify(msg));
		}catch(e){
			ResetSystem();
		}
	}else{
		console.log('error setting up socket');
		ResetSystem();
	}
}







function WatchDogTimerOperations(){
	if(!Comms.watchdog.enable){
		Comms.watchdog.enable    = true;
		Comms.watchdog.lastKick = Helpers.GetEpoch();
		setInterval(function(){CheckTimeOfLastReceived();},1000);
	}else{
		Comms.watchdog.lastKick = Helpers.GetEpoch();
	}
}



function CheckTimeOfLastReceived(){
	
	let now 	= Helpers.GetEpoch();
	let difTime = now - Comms.watchdog.lastKick;
	//console.log('watchdog check: dif'+difTime);
	if(difTime>=Comms.watchdog.TIMEOUT_SECONDS){
		console.log('no update from server for a while!');
		ResetSystem();    //a watchdog of sorts to refresh the screen
	}
}





