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
session_start();
if($_GET["lang"])
	{
	$_SESSION["lang"] = $_GET["lang"];
	if($_GET["redirect"])
		header("Location: ".$_GET["redirect"]);
	else
		header("Location: ".$_SERVER['HTTP_REFERER']);
	};
?>