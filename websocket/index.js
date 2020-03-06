'use strict';
var server = require('ws').Server;
const db = require('./db');
var wss = new server({port: 5001});
var device = null;
console.log("Server now listening on %s:%s", require('ip').address(), wss.address().port);

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
			default:
				ws.terminate();
				break;
		}
	} else {
		console.log("warning: protocol field missing for %s", request.socket.remoteAddress);
	}

	ws.on('message', function(msg){
		try { //to avoid crashing when theres error inparsing
			var in_data = JSON.parse(msg);
			// console.log(in_data);
			switch (in_data.type) {
				case 'colorimeter':
					console.log(in_data.content);
					break;
				case 'push-update':
					// TODO: save new values to db on intervals
					if(in_data.content.type === 'non-realtime'){
						in_data.content.content.lastUpdated = Date.now();
					}
					broadcastToClients(JSON.stringify(
						{
							"type": "update",
							"content": in_data.content
						}
					));
					break;
				case 'request':
					if (in_data.content === "values") {
						
						ws.send(generateUpdateJSON("realtime", getRealtimeValues()));
						ws.send(generateUpdateJSON("non-realtime", getNonRealtimeValues()));
					}
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

				default:
					console.log("Unknown message type:");
					console.log(in_data.content);
			}
		} catch (error) {
			console.log(msg);
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
	ws.send(JSON.stringify({ //reply a status device after a client connects
		type: 'connection',
		content: getDeviceStatus()
	}));
}

function getRealtimeValues() {
	// db.query("SELECT * FROM \"sensor\" ORDER BY timestamp DESC LIMIT 1", (err, res) => {
	// 	if (!err) {
	// 		let data = {}
	// 		data.type				=	"realtime"
	// 		data.water_temp 		=	res.rows[0]["water_temp_aqrm"]
	// 		data.water_level_aqrm	=	res.rows[0]["water_level_aqrm"]
	// 		data.water_level_tank	=	res.rows[0]["water_level_tank"]
	// 		data.air_humidity 		= 	res.rows[0]["air_humidity"]
	// 		data.air_temp 			=	res.rows[0]["air_temp"]
	// 		data.food_available		= 	res.rows[0]["food_available"]
	// 		ws.send(JSON.stringify({"type": "update", "content": JSON.stringify(data)}));
	// 		console.log(JSON.stringify({"type": "update", "content": JSON.stringify(data)}));
	// 	} else {
	// 		console.log("Failed to query realtime values from DB.")
	// 	}
	// })

	return {
		"WaterLevelAquarium": 101,
		"WaterLevelTank": 63,
		"WaterTemp": 69,
		"AirTemp": 10,
		"AirHumidity": 100,
		"Feeds": 100,
	};
}

function getNonRealtimeValues() {
	// db.query("SELECT * FROM \"sensor\" ORDER BY timestamp DESC LIMIT 1", (err, res) => {
	// 	if (!err) {
	// 		let data = {}
	// 		data.type				=	"realtime"
	// 		data.water_temp 		=	res.rows[0]["water_temp_aqrm"]
	// 		data.water_level_aqrm	=	res.rows[0]["water_level_aqrm"]
	// 		data.water_level_tank	=	res.rows[0]["water_level_tank"]
	// 		data.air_humidity 		= 	res.rows[0]["air_humidity"]
	// 		data.air_temp 			=	res.rows[0]["air_temp"]
	// 		data.food_available		= 	res.rows[0]["food_available"]
	// 		ws.send(JSON.stringify({"type": "update", "content": JSON.stringify(data)}));
	// 		console.log(JSON.stringify({"type": "update", "content": JSON.stringify(data)}));
	// 	} else {
	// 		console.log("Failed to query realtime values from DB.")
	// 	}
	// })
	return {
		
		"Alkalinity": {"value": 15, "lastUpdated": Date.now()},
		"Ammonia": {"value": 25, "lastUpdated": Date.now()},
		"Chlorine": {"value": 13, "lastUpdated": Date.now()},
		"Nitrate": {"value": 10, "lastUpdated": Date.now()},
	};
}

function getSettings(){
	db.query("SELECT * FROM \"sensor\" ORDER BY timestamp DESC LIMIT 1", (err, res) => {
		if (!err) {
			let data = {}
			data.fan_speed	 		=	res.rows[0]["water_temp_aqrm"]
			data.light_intensity	=	res.rows[0]["water_level_aqrm"]
			data.water_level_tank	=	res.rows[0]["water_level_tank"]
			ws.send(JSON.stringify({"type": "settings", "content": JSON.stringify(data)}));
			console.log(JSON.stringify({"type": "update", "content": JSON.stringify(data)}));
		} else {
			console.log("Failed to query settings from DB.")
		}
	})
}

// move db query to a separate function...?
function generateUpdateJSON(type, content) {
	return JSON.stringify(
		{
			"type": "update",
			"content": {
				"type": type,
				"content": content,
			}
		}
	);
}

function getDeviceStatus() {
	if(device==null || device.readyState === server.CLOSED){
		return 'device_offline';
	}else{
		return 'device_online';
	}
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