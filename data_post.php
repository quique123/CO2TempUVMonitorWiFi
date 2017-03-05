<?php
$conex = mysql_connect("localhost", "myuser", "mypwd");

if(!$conex){
    die("Couldn't connect to the server...<br />".mysql_error());
}

$db = mysql_select_db("test", $conex);

if(!$db){
    die("Couldn't connect to the database...<br />".mysql_error());
}

$username = $_POST['name'];
$age = $_POST['age'];
$uvindex = $_POST['uvindex'];
$mq2 = $_POST['mq2'];
$sql = "INSERT INTO example (name,age,uvindex,mq2,beer) VALUES ('$username','$age','$uvindex','$mq2','NO')";

$res = mysql_query($sql, $conex);

//echo ($res);
?>
