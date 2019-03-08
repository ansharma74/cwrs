<?php
/*
 * AWS XMS - Online Web Development
 * 
 * Copyright (c) 2010 Cezar Lucan cezar.lucan@aws-dms.com
 * Licensed under GPL license.
 * http://www.aws-dms.com
 *
 * Date: 2010-10-24
 */

function procExecute($command)
	{
	$retValue = 
		array(
			"STDOUT"=>'',
			"STDERR"=>'',
			"SUCCES"=>'', //// 0 succes 1 eroare
			"STATUS"
		     );
	
	$descriptorspec = array(
	   0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
	   1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
	   2 => array("pipe","w")
	);
	
	$process = proc_open("bash", $descriptorspec, $pipes);
		fwrite($pipes[0], $command);
	fclose($pipes[0]);
	
	//STDOUT:
	    while (!feof($pipes[1])) {
	        $retValue["STDOUT"].= fgets($pipes[1], 1024);}
	    fclose($pipes[1]);
	//STDERR:
	    while (!feof($pipes[2])) {
	        $retValue["STDERR"].= fgets($pipes[2], 1024);}
	    fclose($pipes[2]);
	//STATUS
		$retValue["STATUS"] = proc_get_status($process);
	//SUCCES
		$retValue["SUCCES"] = proc_close($process);
	return $retValue;
	}
	
	?>