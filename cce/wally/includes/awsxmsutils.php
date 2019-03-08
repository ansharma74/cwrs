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
//beta3: appConfig

function __autoload($class_name)
{
    require_once 'includes/'. $class_name . '.inc.php';
}

/////////////////////////////////////////
/////////////////UTILITY FUNCTIONS///////
/////////////////////////////////////////
function appConfig($query,$context = FALSE,$configFile = FALSE)
								{
								if(!$configFile)
									return $GLOBALS["CONFIG"]->q('//config/'.$GLOBALS['appName'].$query,$context)->results->item(0)->textContent;
									else
										if(file_exists($configFile))
											{
											$tmpConfig = new awsXML(file_get_contents($configFile));
											return $tmpConfig->q('//config/'.$GLOBALS['appName'].$query,$context)->results->item(0)->textContent;
											}
								}

?>
