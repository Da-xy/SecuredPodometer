<!DOCTYPE html>
<html>
	<head>
		<link rel="stylesheet" href="//code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css">
		<script src="https://code.jquery.com/jquery-1.12.4.js"></script>
  	<script src="https://code.jquery.com/ui/1.12.1/jquery-ui.js"></script>
		<script src="js/datepicker.js"></script>
		<title>Podometer Site</title>
	</head>
	<body>

		<h1>Backend podometer</h1>



		<?php
			require_once('function/function.php');
			

			$today = time();
			$day = date('j', $today);
			$month = date('n', $today);
			$year = date('Y', $today);

			$nbPasToday = nbStepForOneDay('database/data.csv', $day, $month, $year);
			echo "<p>Aujourd'hui vous avez parcouru :</p>";
			echo $nbPasToday."<br>";

		 ?>
		 <p>Vous pouvez consulter une date antérieure en renseignant la date ci-dessous !</p>
		 <form method="post">
			 <input type="text" id="datepicker" name="date">
			 <input type="submit">
		 </form>


		 <?php
		   //&& preg_match('/^(0[1-9]|[12][0-9]|3[01])[- /.](0[1-9]|1[012])[- /.](19|20)\d\d$/', $_POST["date"]) == 1
			 if(isset($_POST) && !empty($_POST) && strlen($_POST["date"]) <=10){
				 $date = explode("/", $_POST["date"]);
				 $day = intval($date[0]);
	 			 $month = intval($date[1]);
	 			 $year = intval($date[2]);

				 $nbPasDate = nbStepForOneDay('database/data.csv', $day, $month, $year);
	 			 echo "<p>A cette date vous avez parcouru :</p>";
	 			 echo $nbPasDate."<br>";
			 }
		 ?>

	</body>
</html>
