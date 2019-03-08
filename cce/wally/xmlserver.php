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

//0.9: namespace fix when saving an xml file XSLT transformer

session_start();
include_once "defaults.php";
include_once "includes/awsxmsutils.php";
include_once "includes/awsiptfeutils.php";

	switch ($_POST["cat"])
		{
		case 'IPTABLESVER':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				$version = procExecute("sudo ".AWS_WALLY_IPTABLES." -V");
				$version["STDOUT"] = str_replace(" ","-",$version["STDOUT"]);
				$version["STDOUT"] = trim($version["STDOUT"],"\r..\n..\t");
				echo $version["STDOUT"];
				}
		break;
		

		case 'IPTABLESGET':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				$out = procExecute("sudo ".AWS_WALLY_IPTABLES_SAVE." | ".AWS_WALLY_IPTABLES_XML);
				echo $out["STDOUT"];
				}
		break;
		
		case 'IPTABLESFLUSH':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				//Reset Default Policies
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P INPUT ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P FORWARD ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P OUTPUT ACCEPT");
				
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P PREROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P POSTROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P OUTPUT ACCEPT");
				
 				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P FORWARD ACCEPT");
 				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P INPUT ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P POSTROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P PREROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P OUTPUT ACCEPT");
				//Flush all rules
				procExecute("sudo ".AWS_WALLY_IPTABLES." -F");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -F");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -F");
				//Erase all non-default chains
				procExecute("sudo ".AWS_WALLY_IPTABLES." -X");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -X");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -X");
				}
		break;
				
		case 'IPTABLESSET':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				//'IPTABLESFLUSH'
				//Reset Default Policies
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P INPUT ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P FORWARD ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -P OUTPUT ACCEPT");
				
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P PREROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P POSTROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -P OUTPUT ACCEPT");
				
 				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P FORWARD ACCEPT");
 				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P INPUT ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P POSTROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P PREROUTING ACCEPT");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -P OUTPUT ACCEPT");
				//Flush all rules
				procExecute("sudo ".AWS_WALLY_IPTABLES." -F");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -F");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -F");
				//Erase all non-default chains
				procExecute("sudo ".AWS_WALLY_IPTABLES." -X");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t nat -X");
				procExecute("sudo ".AWS_WALLY_IPTABLES." -t mangle -X");
				
				$file = ".dumping-iptables.out";
				
				//iptables version
				$version = procExecute("sudo ".AWS_WALLY_IPTABLES." -V");
				$version["STDOUT"] = str_replace(" ","-",$version["STDOUT"]);
				$version["STDOUT"] = trim($version["STDOUT"],"\r..\n..\t");
				
				$xmldoc = new DOMDocument("1.0");
				$xmldoc->loadXML($_POST["data"]);
				$xsl = new DOMDocument;
				$xsl->load('xsl/'.$version["STDOUT"].'.xsl');
				$proc = new XSLTProcessor;
				$proc->importStyleSheet($xsl);
					
				$documentSource = $proc->transformToXML($xmldoc);
					
				if(get_magic_quotes_gpc())
						file_put_contents($file,stripslashes($documentSource));
					else
						file_put_contents($file,$documentSource);
						

			
				$out = procExecute("cat ".$file." | sudo ".AWS_WALLY_IPTABLES_RESTORE);
				unlink($file);
				}
		break;
		
		case 'DATA_IMPORT':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				echo file_get_contents($_POST["location"]);
				}
		break;
		
		case 'DATA_IMPORT':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				echo file_get_contents($_POST["location"]);
				}
		break;
		
		case 'HTML2AWS':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				header("Content-Type: text/xml;");
				$xmldoc = new DOMDocument("1.0");
				$xmldoc->loadHTML(file_get_contents($_POST["location"]));
				$xsl = new DOMDocument;
				$xsl->load('xsl/html2aws.xsl');
				$proc = new XSLTProcessor;
				$proc->importStyleSheet($xsl);
					
				echo $proc->transformToXML($xmldoc);
				}
		break;
		
		case 'SAVE_FILE_FROM_POST':
			if($_SESSION["xmleditoradmin"] == AWS_DESIGNER_ADMIN)
				{
				$file = $_POST["location"];
				
				$documentSource = $_POST["data"];
				
				if(AWS_HTML_XSL_NAMESPACE_FIX && $_POST["namespaceFix"] == "true")
					{
					$xmldoc = new DOMDocument("1.0");
					$xmldoc->loadXML($_POST["data"]);
					$xsl = new DOMDocument;
					
					if($_POST["usethisstylesheet"])
						$xsl->load($_POST["usethisstylesheet"]);
						else
						$xsl->load('xsl/namespacefix-xmldoc.xsl');
						
					$proc = new XSLTProcessor;
					$proc->importStyleSheet($xsl);
					
					$documentSource = $proc->transformToXML($xmldoc);
					}
				
				
				if(get_magic_quotes_gpc())
						file_put_contents($file,stripslashes($documentSource));
					else
						file_put_contents($file,$documentSource);
						
				if($_POST["download_also"]=="true")
					if (file_exists($file)) {
										    header('Content-Description: File Transfer');
										    header('Content-Type: application/octet-stream');
										    header('Content-Disposition: attachment; filename='.basename($file));
										    header('Content-Transfer-Encoding: binary');
										    header('Expires: 0');
										    header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
										    header('Pragma: public');
										    header('Content-Length: ' . filesize($file));
										    ob_clean();
										    flush();
										    readfile($file);
										    exit;
										}
				}
		break;
		case 'LOGIN':
				header("Content-Type: text/xml;");
				
				if(!empty($_POST["user"]) && $_POST["user"] != "User name" &&  !empty($_POST["pass"]))
						{
						if(AWS_DESIGNER_ADMIN == $_POST["user"] && AWS_DESIGNER_PASSWORD == $_POST["pass"])
							{
							echo "<response><login>allowed</login></response>";
							$_SESSION["xmleditoradmin"] = $_POST["user"];
							}
							else 
								echo "<response><login>denied</login></response>";
						}
		break;
		
		case 'LOGOFF':
					unset($_SESSION["xmleditoradmin"]);
					session_unset();
					session_regenerate_id(true);
		break;
		
		}

?>
