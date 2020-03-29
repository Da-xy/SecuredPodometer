<?php
	require_once('../function/function.php');

	$callback = file_get_contents('php://input');
	$callback_tab = json_decode($callback, true);
	$key = read('../database/privatekey.key');


	if($callback_tab["device"] == "1D1A52" && strlen($callback_tab["data"]) <= 24 && preg_match('/^[a-fA-F0-9]+$/', $callback_tab["data"]) == 1){ // Test injection dans le payload
		exec('../exec/dechiffrement '.$callback_tab["data"].' '.$key, $deciphered);
		$data = dateToArray($callback_tab["time"]);
		array_push($data, $callback_tab["device"]);
		array_push($data, $deciphered[0]);
		var_dump($deciphered[0]);
		if(substr($deciphered[0], 4, 12) == "000000000000"){
			registerData('../database/data.csv', $data);
		}
	}

?>
