<?php

	function read ($file_name) {
	  $file    = fopen( $file_name, "r" );
	  $content = "";
	  while(!feof($file)) {
		$content .= fgets($file, 4096);
	  }
	  fclose($file);
	  return $content;
	}

	function write ($file_name, $values, $mode) {
	  $file = fopen($file_name, $mode);
	  fwrite($file,$values);
	  fclose($file);
	}

	function arrayToString($array){
		$chaine = "";
		foreach($array as $e){
			$chaine .= $e."\n";
		}
		return $chaine;
	}

	function registerData($nomfichier, $array){
		$fichier = fopen($nomfichier, 'a');
		fputcsv($fichier, $array, ';');
		fclose($fichier);
	}

	function nbStepForOneDay($nomFichier, $day, $month, $year){
		$nbPas = 0;
		$handle = fopen($nomFichier, 'r');
		while(($data = fgetcsv($handle, 1000, ";")) !== FALSE){
			if(strcmp($day, $data[0]) == 0 && strcmp($month, $data[1]) == 0 && strcmp($year, $data[2]) == 0){
				$nbPas += hexdec(substr($data[7], 0 , 4));
			}
		}
		return $nbPas;
		fclose($handle);
	}

	function dateToArray($time){
		$dateArray = array();
		$dateArray[0] = date('j',$time);
		$dateArray[1] = date('n',$time);
		$dateArray[2] = date('Y',$time);
		$dateArray[3] = date('G',$time);
		$dateArray[4] = date('i',$time);
		$dateArray[5] = date('s',$time);
		return $dateArray;
	}


?>
