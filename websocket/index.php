<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Websocket</title>
</head>
<body>
    <script>
        var sock = new WebSocket("ws://10.0.0.33:5001", 'client');
        // sock.onopen = function(event){
        //     sock.send(JSON.stringify({
        //         type: 'connection',
        //         content: 'client'
        //     }));
        //     alert('Websocket connected');
        // }

        sock.onmessage = function(event){
            console.log(event.data);
            var in_data = JSON.parse(event.data);
            newLog(in_data.content);
        };

        function send(){
            sock.send(JSON.stringify({
                type: 'command',
                content: document.getElementById('msg').value
            }));
            console.log('sending');
        }

        function newLog(text){
            document.getElementById("log").value += "\n"+text;
            document.getElementById("log").scrollTop = document.getElementById("log").scrollHeight 
        }

    </script>

    <input type="number" id='index'>
    <input type="text" id='msg'>
    <button onclick='send()'>send</button>
    <br>
    <textarea name="log" id="log"></textarea>
</body>
</html>
