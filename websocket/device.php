<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <script>
        var sock = new WebSocket("ws://10.0.0.4:5001", "device");
        sock.onopen = function(event){
            sock.send(JSON.stringify({
                type: 'connection',
                content: 'device'
            }));
            alert('Websocket connected');
        }

        sock.onmessage = function(event){
            var in_data = JSON.parse(event.data);
            alert(in_data.content);
            console.log(in_data.content);
        };

    </script>
    
</body>
</html>