<?php

	require_once('../function/function.php');

	header('Content-Type:application/json');

	if ($_SERVER['REQUEST_METHOD']!=='POST'){
	  header("HTTP/1.0 405 Method Not Allowed");
	  die();
	}

	$_POST = json_decode(file_get_contents('php://input'), true);

	if (is_null($_POST['device'])){
	  header("HTTP/1.0 204 No Content");
	  die();
	}
	
	if($callback_tab["device"] != "1D1A52" || strlen($callback_tab["data"]) > 24 || preg_match('/^[a-fA-F0-9]+$/', $callback_tab["data"]) != 1){ // Test injection dans le payload {
		die();
	}

	$key = trim(read('../database/privatekey.key'));
	$newKey = trim(read('../database/newkey.key'));

	exec('../exec/dechiffrement '.$_POST['data'].' '.$key, $rand);
	exec('../exec/dechiffrement '.$_POST['data'].' '.$newKey, $change);



	if(!is_null($_POST['ack'])){
	  if(strcmp($change[0], '4348414e47454f4b') == 0) { // ASCII DE CHANGEOK
		write('../database/privatekey.key', $newKey, 'w');
	  }
	  else {
		exec('../exec/xorString '.$rand[0].' '.$key, $xor);
		exec('../exec/chiffrement '.substr($xor[0],0,16).' '.$key, $newKey1);
		exec('../exec/chiffrement '.substr($xor[0],16,16).' '.$key, $newKey2);


		$newKey = $newKey1[0].$newKey2[0];

		exec('../exec/chiffrement '.$rand[0].' '.$newKey, $message);


		write('../database/newkey.key', $newKey, 'w');

		//8 bytes message
		$payload = array($_POST['device']=>array('downlinkData'=>$message[0]));
		echo json_encode($payload);
	  }
	}

?>
