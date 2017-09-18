<?php header('Content-Type: text/html; charset=ISO-8859-1'); ?>
<?php

$query = $_GET['queryText'] ;
$models = $_GET['models'] ;
$page = $_GET['page'] ;
//echo $query 

if ($query != "") // consult
{

	$results = array();

	$fp = fsockopen("localhost", 3000, $errno, $errstr, 1);
	if (!$fp) {
		echo "Erro: $errstr ($errno)<br>\n";
	} else {
		$msg = $page . ';' . $models . ';' . $query;
		fputs($fp, $msg);
		while (!feof($fp)) {
			$results[] = fgets($fp);
		}
		fclose($fp);
	}

	foreach ($results as $result)
	{
		if (!isset($result) || $result==NULL) continue;
		echo $result;
	}

}else{
	echo "Empty query";
}

?>

