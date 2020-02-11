'use strict';
var server = require('ws').Server;
var wss = new server({port: 5001});
var device;
var device_status = false;

wss.on('connection', function(ws){

	ws.on('message', function(msg){
		try { //to avoid crashing when theres error inparsing
			var in_data = JSON.parse(msg);
			console.log(msg);
			if(in_data.type=='connection'){
				if(in_data.content=='device'){
					device = ws;
					device.on('pong', heartbeat); //set the device  variable is alive to true if theres a reply from the device 
					device.on('close', function(){ //if the device is disconnected properly
						wss.clients.forEach(function each(client){
							if (client !== device) {
								client.send(JSON.stringify({
									type: 'connection',
									content: 'device_offline'
								}));
							}
						});
						console.log('device offline');
					})
					wss.clients.forEach(function each(client){ //broadcast a message if the device is online
						if (client !== device) {
							client.send(JSON.stringify({
								type: 'connection',
								content: 'device_online'
							}));
						}
					});
					console.log('device online');
				}else{
					var device_status;
					if(device==null || device.readyState === server.CLOSED || !device_status){
						device_status = 'device_offline';
					}else{
						device_status = 'device_online';
					}
					ws.send(JSON.stringify({ //reply a status device after a client connects
						type: 'connection',
						content: device_status
					}));
					console.log('client connected');
				}
			}else if(in_data.type=='command'){
				if(device==null || device.readyState === server.CLOSED){ //checks if the device is online before broadcasting a command
					msg = JSON.stringify({
						type: 'connection',
						content: 'device_offline'
					});
				}
				wss.clients.forEach(function each(client){
						if(client != ws) client.send(msg);
				});

				if(in_data.content == 'off'){ //command to turn off the device websocket connection
					device.close();
					console.log('shutting down the device');
				}
			}	
		} catch (error) {
			console.log(error.message);
		}
		
	})
});

setInterval(function(){ //checks if device is still online, this is useful when the device suddenly turned off
	if (device != null) {
		if(device.isAlive ==false && device_status != device.isAlive){
			wss.clients.forEach(function each(client){
				if (client !== device) {
					client.send(JSON.stringify({
						type: 'connection',
						content: 'device_offline'
					}));
				}
			});
			return device.terminate();
		}
		device.isAlive=false;
		device.ping(); //ping the device to check its online
		device_status = device.isAlive;
	}
},3000);

function heartbeat(){ //set the flag to true if device has a reply after pinging
	this.isAlive = true;
}