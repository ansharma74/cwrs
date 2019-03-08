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

//TESTING XSL PROCESSOR LOADED
if(!extension_loaded("xsl")) die ("XSL Extension not loaded.<br/>Please install php xslt extension");

ini_set("display_errors","Off");

ini_set("error_reporting","~E_NOTICE");

ini_set("magic_quotes_gpc","Off");

ini_set('include_path', ini_get('include_path').':'.dirname($_SERVER['SCRIPT_FILENAME']). PATH_SEPARATOR ."includes");

//HOME APPLICATION
define('AWS_HOME',						"templates/index.xml");

define('AWS_DEFAULTS_LOADED', 			TRUE);


//DESIGNER's USER NAME AND PASSWORD
define('AWS_DESIGNER_ADMIN', 			"admin");

define('AWS_DESIGNER_PASSWORD', 		"admin");


//DESIGNER THEME - FULL LIST IN CSS FOLDER
define('AWS_DESIGNER_THEME',	 		"redmond");

define('AWS_HTML_XSL_NAMESPACE_FIX',	TRUE);


//LOG LAMDA FUNCTIONS TO BE RECORDED - FOR TESTING ONLY, SLOWS DOWN THE APPLICATION
//IF ENABLED SEE THE FILES IN log FOLDER
define('AWS_DEBUG_ALL_LAMBDA',	FALSE);

//THE STREAM TO USE FOR LOG - REMOVE compress.zlib:// IF DO NOT WANT TO BE ARCHIVED
define('AWS_DEBUG_ALL_LAMBDA_FILENAME',	"compress.zlib://log/".session_id());


//FOR MATCH AND MATCHITERATOR DIRECTIVES
define("AWS_ITERATOR_MATCH_PREFIX",'/\{-\{');

define("AWS_ITERATOR_MATCH_SUFFIX",'\}-\}/');


//CACHE FOLDER
//define('AWS_CACHE_LOCATION',	 		"cache".DIRECTORY_SEPARATOR.session_id());
define('AWS_CACHE_LOCATION',	 		"cache");


//404 ERROR APPLICATION
define('AWS_ERROR_404',	 		"templates/404.xml");

//////////////////////////////////////////////////////
//IPTABLES FRONTEND///////////////////////////////////
//////////////////////////////////////////////////////
define('AWS_WALLY_IPTABLES',			"/usr/sbin/iptables");
define('AWS_WALLY_IPTABLES_SAVE', 		"/usr/sbin/iptables-save");
define('AWS_WALLY_IPTABLES_RESTORE', 	"/usr/sbin/iptables-restore");
define('AWS_WALLY_IPTABLES_XML', 	"/usr/bin/iptables-xml");

?>
