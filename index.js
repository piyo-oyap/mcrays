'use strict';
var server = require('ws').Server;
var wss = new server({port: 5001});
var device;
var device_status = false;

wss.on('connection', function(ws){

	ws.on('message', function(msg){
		try {
			var in_data = JSON.parse(msg);
			console.log(msg);
			if(in_data.type=='connection'){
				if(in_data.content=='device'){
					device = ws;
					device.on('pong', heartbeat);
					device.on('close', function(){
						wss.clients.forEach(function each(client){
							if (client !== ws) {
								client.send(JSON.stringify({
									type: 'connection',
									content: 'device_offline'
								}));
							}
						});
						console.log('device offline');
					})
					wss.clients.forEach(function each(client){
						if (client !== ws) {
							client.send(JSON.stringify({
								type: 'connection',
								content: 'device_online'
							}));
						}
					});
					console.log('device online');
				}else{
					var device_status;
					if(device==null || device.readyState === server.CLOSED){
						device_status = 'device_offline';
					}else{
						device_status = 'device_online';
					}
					ws.send(JSON.stringify({
						type: 'connection',
						content: device_status
					}));
					console.log('client connected');
				}
			}else if(in_data.type=='command'){
				if(device==null || device.readyState === server.CLOSED){
					msg = JSON.stringify({
						type: 'connection',
						content: 'device_offline'
					});
				}
				wss.clients.forEach(function each(client){
						client.send(msg);
				});

				if(in_data.content == 'off'){
					device.close();
					console.log('shutting down the device');
				}
			}	
		} catch (error) {
			console.log(error.message);
		}
		
	})
});

setInterval(function(){
	if (device != null) {
		device.ping();
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
		device_status = device.isAlive;
	}
},3000);

function heartbeat(){
	this.isAlive = true;
}