<?php

// $conn = new PDO('pgsql:host=10.0.0.33;dbname=mcrays_db', 'mcrays', 'aquaponics');
// $result = $conn->query('select * from user_acc');
$conn = pg_connect("host=10.0.0.33 dbname=mcrays_db user=mcrays password=aquaponics");
$result = pg_query($conn, $_GET['q']);
$row = pg_fetch_assoc($result);
$json = json_encode($row);
echo $json;
// 
?>