<?php
include ("utils.php");
include_once("JSON.php");
$json = new Services_JSON();

$link = mysql_pconnect("localhost", "myusr", "mypwd") or die("Could not connect");
mysql_select_db("test") or die("Could not select database");


$sqlChart_Q=("SELECT age,unix_timestamp(timestamp) FROM someTable WHERE id > 2500 ORDER BY unix_timestamp(timestamp) ASC");
   $sqlChart_R=mysql_query($sqlChart_Q);
   while($arrChart_ROW=mysql_fetch_row($sqlChart_R)) {
      $time=(int)$arrChart_ROW[1]*1000;
	  $age=(float)$arrChart_ROW[0]; 
      $arrChart[]=array($time, $age);
   }
echo json_encode($arrChart);
mysql_close($link);
?>
