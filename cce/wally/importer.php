<?php
/*
 * XMS - Online Web Development
 * 
 * Copyright (c) 2010 Cezar Lucan cezar.lucan@aws-dms.com
 * Licensed under GPL license.
 * http://www.aws-dms.com
 *
 * Date: 2010-10-24
 */

function isJS($fisier)
{
$sir=explode('.',$fisier);
if (strtolower($sir[sizeof($sir)-1])=='js') return true;
	else return false;
}

function isCSS($fisier)
{
$sir=explode('.',$fisier);
if (strtolower($sir[sizeof($sir)-1])=='css') return true;
	else return false;
}


$explodedFile = explode('.',$_GET["what"]);
unset($explodedFile[sizeof($explodedFile)-1]);

if(isJS($_GET["what"]))
	{
	  
	  if(file_exists(implode(".", $explodedFile).".psj"))
	  	{
		  header("Cache-Control: no-store, no-cache");
		  header("Pragma: no-cache");
		  header("Content-Type: text/javascript");
		  header("Content-Encoding: gzip");
		  echo join('',file(implode(".", $explodedFile).".psj"));
	  	}
		elseif(file_exists(implode(".", $explodedFile)."-min-packed.js"))
			{
			ob_start( 'ob_gzhandler' );
			header("Cache-Control: no-store, no-cache");
		  	header("Pragma: no-cache");
			header("Content-Type: text/javascript");
			header("Content-Encoding: gzip");
			echo join('',file(implode(".", $explodedFile)."-min-packed.js"));			
			ob_end_flush();	
			}
		elseif(file_exists(implode(".", $explodedFile)."-min.js"))
			{
			ob_start( 'ob_gzhandler' );
			header("Cache-Control: no-store, no-cache");
		  	header("Pragma: no-cache");
			header("Content-Type: text/javascript");
			header("Content-Encoding: gzip");
			echo join('',file(implode(".", $explodedFile)."-min.js"));			
			ob_end_flush();	
			}
		elseif(file_exists($_GET["what"]))
			{
			header("Cache-Control: no-store, no-cache");
		  	header("Pragma: no-cache");
			header("Content-Type: text/javascript");
			echo join('',file($_GET["what"]));
			}	  
	}

?> 
