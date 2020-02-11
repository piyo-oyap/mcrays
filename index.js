'use strict';
var server = require('ws').Server;
var wss = new server({port: 5001});
var device = null;
console.log("Server now listening on %s:%s", wss.address().address, wss.address().port);

wss.on('connection', function(ws, request){
	if ("sec-websocket-protocol" in request.headers) {
		ws.on('pong', heartbeat); //set the device  variable is alive to true if theres a reply from the device 
		switch (request.headers["sec-websocket-protocol"]) {
			case "client":
				broadcastDeviceStatus(ws);
				ws.on('close', function(){
					console.log("%s | client disconnected", request.socket.remoteAddress);
				});
				console.log("%s | client connected", request.socket.remoteAddress);
				break;
			case "device":
				device = ws;
				
				device.on('close', function(){ //if the device is disconnected properly
					broadcastToClients(JSON.stringify({
						type: 'connection',
						content: 'device_offline'
					}));
					console.log('device offline');
				})
				broadcastToClients(JSON.stringify({
					type: 'connection',
					content: 'device_online'
				}));
				console.log('%s | device online', request.socket.remoteAddress);
				break;
		}
	} else {
		console.log("warning: protocol field missing for %s", request.socket.remoteAddress);
	}

	ws.on('message', function(msg){
		try { //to avoid crashing when theres error inparsing
			var in_data = JSON.parse(msg);
			console.log(msg);
			switch (in_data.type) {
				case 'update':
					
					break;
				case 'command':
					if(device==null || device.readyState === server.CLOSED){ //checks if the device is online before broadcasting a command
						msg = JSON.stringify({
							type: 'connection',
							content: 'device_offline'
						});
					}
					broadcastToAll(msg);

					break;
			}
		} catch (error) {
			console.log(error.message);
		}
		
	})
});

function broadcastToAll(msg){
	wss.clients.forEach(function(client){
			client.send(msg);
	});
}

function broadcastToClients(msg){
	wss.clients.forEach(function(client){
		if(client != device){
			client.send(msg);
		}
	});
}

function broadcastDeviceStatus(ws) {
	var device_status_response;
	if(device==null || device.readyState === server.CLOSED){
		device_status_response = 'device_offline';
	}else{
		device_status_response = 'device_online';
	}
	ws.send(JSON.stringify({ //reply a status device after a client connects
		type: 'connection',
		content: device_status_response
	}));
}

setInterval(function(){ //checks if device is still online, this is useful when the device suddenly turned off
	wss.clients.forEach(function(ws){
		if (ws.isAlive === false) {
			if (device === ws) device = null;
			return ws.terminate();
		}

    	ws.isAlive = false;
    	ws.ping();
	})
},3000);

function heartbeat(){ //set the flag to true if device has a reply after pinging
	this.isAlive = true;
}