<?php
	$host = "sql.serwer2064773.home.pl"; //podaj nazwę serwera, na którym znajduje się baza danych
	$user = "32367870_milionerzy"; //podaj nazwę loginu, służącego do połączenia się z bazą danych
	$password = "Milionerzy12!"; //podaj hasło potrzebne do połączenia się z bazą
	$db = "32367870_milionerzy"; //podaj nazwę bazy danych, z którą chcesz się połączyć
	$connection = mysqli_connect($host, $user, $password);
	mysqli_query($connection, "SET CHARSET utf8");
	mysqli_query($connection, "SET NAMES 'utf8' COLLATE 'utf8_polish_ci'");
	mysqli_select_db($connection, $db);
					
	
	$sprawdzMaxId = $_GET['checkId'];
	$tabela = $_GET['table'];
	$question = $_GET['question'];
	$wartosc = 0;
	
	switch ($tabela) {
		case 1:
			$wartosc = 500;
			break;
		case 2:
			$wartosc = 1000;
			break;
		case 3:
			$wartosc = 2000;
			break;
		case 4:
			$wartosc = 5000;
			break;
		case 5:
			$wartosc = 10000;
			break;
		case 6:
			$wartosc = 20000;
			break;
		case 7:
			$wartosc = 40000;
			break;
		case 8:
			$wartosc = 75000;
			break;
		case 9:
			$wartosc = 125000;
			break;
		case 10:
			$wartosc = 250000;
			break;
		case 11:
			$wartosc = 500000;
			break;
		case 12:
			$wartosc = 1000000;
			break;
			
		default:
			break;
	}
	
	$tabela = "Pytania_za_".$wartosc;
	
	if ($sprawdzMaxId) {
		$zapytanie = mysqli_query($connection, "SELECT * FROM Pytania_za_500 WHERE idp=(SELECT MAX(idp) FROM Pytania_za_500)");
		$wynik = mysqli_fetch_assoc($zapytanie);
		$id = $wynik['idp'];
		echo '<'.$id.'>';
	}
	else {
		$zapytanie = mysqli_query($connection, "SELECT * FROM $tabela WHERE idp='$question'");
		$wynik = mysqli_fetch_assoc($zapytanie);
		$pytanie = $wynik['tresc'];
		$a = $wynik['a'];
		$b = $wynik['b'];
		$c = $wynik['c'];
		$d = $wynik['d'];
		$poprawna = $wynik['poprawna'];
		echo '<'.$pytanie."\n".$a."\n".$b."\n".$c."\n".$d."\n".$poprawna.'>';
	}
?>