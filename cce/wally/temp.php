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
//2.0 beta-4:	fixed to look for local names only to avoid namespace issues
//2.0 beta-3.1:	fixed call time deprecated
//2.0 beta-3:  	old parsers for relace and runhere re-introduced - replace did not worked on the new parser
//2.0 beta-2:  	#php directive - BUG do not copies the text nodes!!!!
//2.0 beta-2:  	#php directive fixed to match php target sibling only
//2.0 beta-2:  	#php directive
//2.0 beta-1:  	advancedimport/data/child[&type=function] - executes the #pi inside
//2.0 beta-1:  	$GLOBALS["HTMLPARSER"] now is awsXML = kept the name for backward compatibility
//2.0 beta-1:  	USING #PI for php code!!!!
//2.0 alpha-5:	app[@outputxsl] - default is xsl/aws2html.xsl for html app; when attribute removed > xml application
//2.0 alpha-5:  introduced #pi exec = exec except it does not accept return child directive
//2.0 alpha-5:  aws #pi - to replace "runhere" and "replace" directives; for compatibility these are the same as php #pi
//2.0 alpha-5:  [@importashtml] attr = for import and advancedimport - when importing HTML source from a html document
//2.0 alpha-5:  [@importashtml] attr - xmlimport and advancedxmlimport removed - functionality preserved with import and advanced import
//2.0 alpha-2:  directives not in "content" (imported) use text for php code and NOT CDATA
//2.0 alpha-1:  php and javascript code moved from text content to CDATA section
//1.0-beta17:	BUG: doXMLImport was using awsHTML - changed to awsXML
//1.0-beta16:	filters for import directives (import, xmlimport, adv...,...)
//1.0-beta15:	case directive: moved before imports, just after content generation: allows use of $GLOBALS["BROWSER"]->Name in "filter" to generate browser related content;
//1.0-beta15:	case directive / processCases bug fixed: not using default option when filter result not found in options list
//1.0-beta14:	domiterator filter: [@modtype="append"] or [@modtype="prepend"]; skip attribute has to be specified also, otherwise will first replace everything then append/prepend again the field value
//1.0-beta13:	case directive
//1.0-beta11:	import/advImport/xmlImport/advXmlImport - eachnode - if return true will accept and add that element, else nope (see docs.ex.import.1.xml, ex 5); to check for all directives
//1.0-beta11:	use/check($doc,$el) - fixed
//1.0-beta10:	cache system: clearcache when 0 will always renew the source; set to FALSE will never renew it
//1.0-beta9: cache system for import / xmlimport / advancedImport / advancedXmlImport and loadRemoteTemplate
//1.0-beta9: remotetemplate works
//1.0-beta8: dirty output fixed, except awsCommonXml->each
//1.0-beta8: /app[outputxsl=local_xsl]: using a custom xstyle for document output
//1.0-beta7: dirty output fixes
//1.0-beta6: remote Template - working code but no output: TODO;
//1.0-beta6: lang fixed from beta 5;
//1.0-beta5: $node->nodeName => $node->localName
//1.0-beta5: /app[outputdisabled=TRUE] - will disable output of the document; for console and blind applications
//1.0-beta5: HTML DOCUMENT BUILD WITH XSL
//1.0-beta4: setTitle
//1.0-beta3: domiterator/eachnamedreference/referencenamenode - will execute the code if a eachnamedreference child with the same name will be found;
//1.0-beta3: domiterator/eachnamedreference/referencenamenode - similar with eachreference but executes code for the named reference only;
//1.0-beta1: exec($parentNode) - same as runhere
//beta13: domiterator bug - before cloning the unit and run translators and callback on the same object; now creating a new object from the unit's source and run callbacks and translators on new objects
//beta13: matchiterator is using descendant-or-self for dom query in looking for unit
//beta13: domiterator is using descendant-or-self for inquiry the dom looking for unit and references
//beta13: domiterator/eachreference(&$el,$label,$value,$recordset)
//beta13: replace - function will be called with parentNode as parameter (like runhere)
//beta13: matchiterator/norecords
//beta13: domiterator/norecords
//beta13: namespaces fixed with XSLT transformer; can be disabled from defaults.php/AWS_HTML_XSL_NAMESPACE_FIX; default TRUE;
//beta13: init and init/return - executed after use and result is put in GLOBALS["APPINIT"][returnDirectiveText]
//beta13: domiterator - unit/reference[skip] - will skip to replace content of that node
//beta13: //app/parsers
//beta13: user directives
// these will only be executed for HTMLPARSER and only after the system defined directives
// rules added in designer
//        parsers.xml for global user directives (will be executed for any application);
//        //app/parsers - for APP related user directives
//beta12: domiterator/eachreference; unit can be any descendant that has unit attribute; reference has attribute reference=label_of_array
//beta12: matchiterator - unit can be now any descendant that has unit attribute, not only a direct children
//beta12: import/eachnode only tested with import; TODO test with others
//beta12: import/eachnode, xmlimport/eachnode,...adv/eachnode; eachnode($el) where $el is each DOMelement of results after importing in document
//beta12: import/runfirst, xmlimport/runfirst,...adv/runfirst; runfirst($el) where $el -DOMNode al directivei
//beta12: import, import, xmlimport .... $doc,$el as parameters for check and filter; $el-DOMNode
//beta11: userParsers
//beta11: parsers plugins support
//beta10: code enhancements
//beta9: BROWSER global variable 
//beta9: Browser class
//beta9: appConfig moved to awsxmsutils
//beta9: use[check]
//beta8: use[source,xpath,where] directive added
//beta8: all directives to lowercase - this is the reason for import failures


session_start();
include_once 'defaults.php';
include_once 'includes/awsxmsutils.php';

//VERSION
define("AWS_TEMPLATE_ENGINE",'2.0-beta4');

$errLogMessage = "";

function _create_function(&$e,$args,$ft)
						{
						if($_GET["use"])
							{
							if(!file_exists($_GET["use"]) || !is_file($_GET["use"]))
								$templateFile = AWS_ERROR_404;
								else 
								$templateFile = $_GET["use"];
							}
						else
							$templateFile = AWS_HOME;
							
						if(AWS_DEBUG_ALL_LAMBDA)
							{
							if($e)
								$GLOBALS["errLogMessage"] .= "--\n".date("Y-m-d-H-i")."\t LAMBDA FUNCTION: ".$e->nodeName."\t ".$templateFile.":".$e->getLineNo()."\t xpath:".$e->getNodePath()."\t DocumentElement: ".$e->ownerDocument->documentElement->nodeName."\n".$ft."\n\n";
							else 
								$GLOBALS["errLogMessage"] .= "--\n".date("Y-m-d-H-i")."\t LAMBDA FUNCTION: inline\t line:inline\t xpath:inline\n".$ft."\n\n";
							}
							
						$toRet = create_function($args,$ft);
						
						if($toRet)
							return $toRet;
						else						
							{
							if($e)
								$GLOBALS["errLogMessage"] = date("Y-m-d-H-i")."\t CREATE LAMBDA FUNCTION ERROR: ".$e->nodeName."\t ".$templateFile.":".$e->getLineNo()."\t xpath:".$e->getNodePath()."\n".$ft."\n\n";
							else 
								$GLOBALS["errLogMessage"] = date("Y-m-d-H-i")."\t CREATE LAMBDA FUNCTION ERROR: inline\t line:inline\t xpath:inline\n".$ft."\n\n";
							
							file_put_contents(AWS_DEBUG_ALL_LAMBDA_FILENAME.".ERR.gz",$GLOBALS["errLogMessage"],FILE_APPEND);
							}
						}


/////////////////////////////////////////
/////////////////CALLBACKS///////////////
/////////////////////////////////////////

function getAppName(&$el)
						{						
							$GLOBALS["appName"] = $el->textContent;
						}
							
function processCases(&$el)
							{
							$found = FALSE;
								
							$defaultNode = FALSE;
							
							$functionText = "";
							
							if(strtolower($el->localName) == "case" && $el->nodeType ==1)
								{
								if($el->hasChildNodes())
									foreach($el->childNodes as $child)
										if($child->nodeType == 1 && $child->localName == "filter")
											foreach($child->childNodes as $filterChild)
												if($filterChild->nodeType == 7)
													$functionText = $filterChild->data;
											
								$toRun = _create_function($el,'$el',$functionText);
								
								$selected = $toRun($el);
								
								if(!$selected) $selected = "default";
								
								$docFragment = $el->ownerDocument->createDocumentFragment();
								
								if(is_string($selected))
									if($el->hasChildNodes())
										foreach($el->childNodes as $childToCheck)
											{
											if($childToCheck->nodeType == 1 && $childToCheck->localName === "default")
												$defaultNode = $childToCheck;
												
											if($childToCheck->nodeType == 1 && $childToCheck->localName === trim($selected))
												{
												$found = TRUE;
												
												foreach($childToCheck->childNodes as $cn)
													$docFragment->appendChild($cn->cloneNode(TRUE));													
												}
											}
								
								if(!$found && $defaultNode)
									foreach($defaultNode->childNodes as $dcn)
										$docFragment->appendChild($dcn->cloneNode(TRUE));



								//inlocuim elementul import cu fragmentul generat
								$el->parentNode->replaceChild($docFragment,$el);
								}
							}
						
function doFilterImport(&$el)
						{
						$hasCheck = false;
						//file_put_contents("imp",file_get_contents(imp).$el->localName."\n");
						//daca nu avem sursa  folosim drept sursa default template-ul
						if($el->hasAttribute("source"))
							$documentFrom = new awsXML(file_get_contents($el->getAttribute("source")));
							
							//doar in cazul in care se da xpath facem modificarile
							if($el->hasAttribute("xpath"))
								{
								$xpath = $el->getAttribute("xpath");
	
										//creez fragmentul de document in HTML-ul nostru
										$docFragment = $el->ownerDocument->createDocumentFragment();
										
										$documentFrom->q($xpath);
										
										foreach($documentFrom->results as $result)
											{
											$newnode = $el->ownerDocument->importNode($result, TRUE);
											$docFragment->appendChild($newnode);
											}
											
										if($el->hasChildNodes())
											foreach($el->childNodes as $cn)
												if($cn->nodeType==1)
													switch($cn->localName)
													{
													case "check":
																if($cn->hasChildNodes())
																	foreach($cn->childNodes as $child)
																		if($child->nodeType == 7)
																				$functionText = $child->data;
			
																$toRun = _create_function($el,'$doc,$el',$functionText);
																$checkedFunctionReturnValue = $toRun($documentFrom,$el);
																
																$hasCheck = true;
													break;
													}
													
										if(!$hasCheck || $hasCheck && $checkedFunctionReturnValue)
											{
											//inlocuim elementul import cu fragmentul generat
											if($el->hasAttribute("where"))
												{
												$last = false;
												$pathMembers = explode("/",$el->getAttribute("where"));
												if(sizeof($pathMembers)>0)
													{
													foreach($pathMembers as $k=>$member)
														{
														if($member)
															if(!$last)
																$last = $el->ownerDocument->createElement($member," ");
																else
																	{
																	$newChild = $el->ownerDocument->createElement($member," ");
																	$last->appendChild($newChild);
																	$last = $newChild;
																	}
														}
													$last->appendChild($docFragment);
													$el->parentNode->replaceChild($last,$el);
													}
												}
											else
												$el->parentNode->replaceChild($docFragment,$el);
											}
								
								}
							
						
						}

						
function loadRemoteTemplate(&$el)
						{
						if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
										{
										case "runfirst":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
															if($child->nodeType == 7)
																	$functionText = $child->data;
													$toRun = _create_function($cn,'$el',$functionText);
													$toRun($el);
										break;
										}

							

						//doar in cazul in care se da xpath facem modificarile
						if($el->hasAttribute("source"))
							{
							if($el->hasAttribute("cache") && $el->getAttribute("cache")=="enabled")
								{
								if(!is_dir(AWS_CACHE_LOCATION))
									mkdir(AWS_CACHE_LOCATION,0777,TRUE);
									
								if($el->hasAttribute("clearcache"))
									if(file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
										if($el->getAttribute("clearcache")!="FALSE" && time()-filemtime(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"))>=$el->getAttribute("clearcache"))
											unlink(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
									
								if(!file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
									file_put_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"),file_get_contents($el->getAttribute("source")));
									
									$sursaDefaultImport = file_get_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
								}
								else
									$sursaDefaultImport = file_get_contents($el->getAttribute("source"));
									
							if($el->hasAttribute("importasxml"))
								$documentFrom = new awsXML($sursaDefaultImport);
								else
								$documentFrom = new awsHTML($sursaDefaultImport);
							/////////////////
							//import/filter//
							/////////////////
							if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
											{
											case "filter":
														if($cn->hasChildNodes())
															foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
														$toRun = _create_function($cn,'$doc,$el',$functionText);
														$toRun($documentFrom,$el);
											break;
											case "check":
														if($cn->hasChildNodes())
															foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
	
														$toRun = _create_function($cn,'$doc,$el',$functionText);
														$checkedFunctionReturnValue = $toRun($documentFrom,$el);
														
														$hasCheck = true;
											break;
											case "finally":
														if($cn->hasChildNodes())
															foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
	
														$withResults = _create_function($cn,'$el',$functionText);
											break;
											}

							if($hasCheck)
								{
								//daca functia returneaza o alta valoare in afara de false
								if($checkedFunctionReturnValue)
									//execut acelasi cod ca atunci cand nu avem un check pe acest element
									{
									$documentTo = new awsXML($documentFrom->content("xsl/html2aws.xsl"));

									if($el->hasAttribute("xpath") && function_exists($withResults))
											$documentTo->q($el->getAttribute("xpath"))->each($withResults);
										
									$el->parentNode->replaceChild($el->ownerDocument->importNode($documentTo->doc->documentElement,TRUE),$el);					
									}
									else
										//in acest caz inlocuiesc element(import aici) cu nimic :D
										{
										$docFragment = $el->ownerDocument->createDocumentFragment();
										//adaugam sursa nodului la fragment
										$docFragment->appendXML(" ");
										
										//inlocuim elementul import cu fragmentul generat
										$el->parentNode->replaceChild($docFragment,$el);										
										}
								}
								else
									{
									$documentTo = new awsXML($documentFrom->content("xsl/html2aws.xsl"));

									if($el->hasAttribute("xpath") && function_exists($withResults))
											$documentTo->q($el->getAttribute("xpath"))->each($withResults);
										
									$el->parentNode->replaceChild($el->ownerDocument->importNode($documentTo->doc->documentElement,TRUE),$el);
									}
							
							}
						}
						
						
function doImport(&$el,&$target = FALSE)
						{
						if($target && gettype($target) == "object")
							$targetNodeInDoc = $target;
						else $targetNodeInDoc = $el;
							
						if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
										{
										case "runfirst":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;

															
													$toRun = _create_function($cn,'$el',$functionText);
													$toRun($el);
										break;
										}
						//daca nu avem sursa  folosim drept sursa default template-ul
						if($el->hasAttribute("source"))
							{
							if($el->hasAttribute("cache") && $el->getAttribute("cache")=="enabled")
								{
								if(!is_dir(AWS_CACHE_LOCATION))
									mkdir(AWS_CACHE_LOCATION,0777,TRUE);
									
								if($el->hasAttribute("clearcache"))
									if(file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
										if($el->getAttribute("clearcache")!="FALSE" && time()-filemtime(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"))>=$el->getAttribute("clearcache"))
											unlink(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
									
								if(!file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
									file_put_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"),file_get_contents($el->getAttribute("source")));
									
									$sursaDefaultImport = file_get_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
								}
								else
									$sursaDefaultImport = file_get_contents(html_entity_decode($el->getAttribute("source")));
							
							if($el->hasAttribute("importashtml"))
								$documentFrom = new awsHTML($sursaDefaultImport);
								else
								$documentFrom = new awsXML($sursaDefaultImport);
							}
							
							else
								$documentFrom = $GLOBALS["TEMPLATE"];
						
						
						//doar in cazul in care se da xpath facem modificarile
						if($el->hasAttribute("xpath"))
							{
							$xpath = $el->getAttribute("xpath");
							
							/////////////////
							//import/filter//
							/////////////////
							if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
										{
										case "filter":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
															
													$toRun = _create_function($cn,'$doc,$el',$functionText);
													$toRun($documentFrom,$el);
										break;
										case "check":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;


													$toRun = _create_function($cn,'$doc,$el',$functionText);
													$checkedFunctionReturnValue = $toRun($documentFrom,$el);
													
													$hasCheck = true;
										break;
										case "eachnode":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;


													$withResults = _create_function($cn,'$el',$functionText);
										break;
										}

							if($hasCheck)
								{
								//daca functia returneaza o alta valoare in afara de false
								if($checkedFunctionReturnValue)
									//execut acelasi cod ca atunci cand nu avem un check pe acest element
									{
									//creez fragmentul de document in HTML-ul nostru
									$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
									
									$documentFrom->q($xpath);

									
									foreach($documentFrom->results as $result)
										{
										$newnode = $targetNodeInDoc->ownerDocument->importNode($result, TRUE);

										if(function_exists($withResults))
											{
											if($withResults($newnode))
												$docFragment->appendChild($newnode);
											}
											else $docFragment->appendChild($newnode);
										}
									
									$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);				
									}
									else
										//in acest caz inlocuiesc element(import aici) cu nimic :D
										{
										$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
										//adaugam sursa nodului la fragment
										$docFragment->appendXML(" ");
										
										//inlocuim elementul import cu fragmentul generat
										$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);										
										}
								}
								else
									{
									//creez fragmentul de document in HTML-ul nostru
									$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
									
									//iau sursele nodului returnat de interogare
									$interogationSource = "";
									
									$res = "";
									$documentFrom->q($xpath);
									
									foreach($documentFrom->results as $result)
										{
										$newnode = $targetNodeInDoc->ownerDocument->importNode($result, TRUE);
										
										if(function_exists($withResults))
											{
											if($withResults($newnode))
												$docFragment->appendChild($newnode);
											}
											else $docFragment->appendChild($newnode);
										}
										
									//inlocuim elementul import cu fragmentul generat
									$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);
									}
							
							}
						}
						
function doAdvancedImport(&$el,&$target = FALSE)
						{
						if($target && gettype($target) == "object")
							$targetNodeInDoc = $target;
						else $targetNodeInDoc = $el;
						
						if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
										{
										case "runfirst":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
															
													$toRun = _create_function($cn,'$el',$functionText);
													$toRun($el);
										break;
										}
						
						$data = "";
						$options = "";
						//
						
							if($el->hasChildNodes())
								foreach($el->childNodes as $ch)
									if($ch->nodeType==1)
										switch($ch->localName)
										{
										case "data":
											//POST and GET vars
											if($ch->hasChildNodes())
												foreach($ch->childNodes as $dataNode)
													if($dataNode->nodeType == 1)
														{
														if($dataNode->getAttribute("type") == "function")
															{
															foreach($dataNode->childNodes as $ChildDataNode)
																if($ChildDataNode->nodeType == 7)
																	{
																	$dataChildfunctionText = $ChildDataNode->data;
																	$toRun = _create_function($cn,'$el',$dataChildfunctionText);
																	//apel functie in 'data' cu DOMElem al 'data' pt acces catre advancedImport
																	//dar si siblings al nodului curent in data
																	$data[$dataNode->localName] = $toRun($ch);
																	}
															}
														else
															$data[$dataNode->localName] = $dataNode->textContent;
														}
										break;
										
										default:
										//toate celelalte http, ftp sau socket
											if($ch->hasAttributes())
												foreach($ch->attributes as $attr)
													$options[$ch->localName][$attr->name] = $attr->value;
										break;
										}
										
						//creez http query
						if(sizeof($data)>0)
							$postdata = http_build_query($data);
							
						//caut in optiuni http sau ftp sau orice in afara de socket
						//si adaug postdata ca si key content
						foreach($options as $k=>$v)
							if(strtolower($k) != "socket")
								$options[$k]["content"] =  $postdata;
							
						//creez context doar daca am optiuni
						if(sizeof($options)>0)
							$context  = stream_context_create($options);
							
							
						//daca nu avem sursa  folosim drept sursa default template-ul
						if($el->hasAttribute("source"))
							{
							if($el->hasAttribute("cache") && $el->getAttribute("cache")=="enabled")
								{
								if(!is_dir(AWS_CACHE_LOCATION))
									mkdir(AWS_CACHE_LOCATION,0777,TRUE);
									
								if($el->hasAttribute("clearcache"))
									if(file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
										if($el->getAttribute("clearcache")!="FALSE" && time()-filemtime(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"))>=$el->getAttribute("clearcache"))
											unlink(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
									
								if(!file_exists(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage")))
									file_put_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"),file_get_contents($el->getAttribute("source"), false, $context));
									
									$sursaDefaultImport = file_get_contents(AWS_CACHE_LOCATION.DIRECTORY_SEPARATOR.$el->getAttribute("cachestorage"));
								}
								else
									$sursaDefaultImport = file_get_contents(html_entity_decode($el->getAttribute("source")), false, $context);
							
							if($el->hasAttribute("importashtml"))
								$documentFrom = new awsHTML($sursaDefaultImport);
								else
								$documentFrom = new awsXML($sursaDefaultImport);
							}
						
						
						//doar in cazul in care se da xpath facem modificarile
						if($el->hasAttribute("xpath"))
							{
							$xpath = $el->getAttribute("xpath");
							
							/////////////////
							//import/filter//
							/////////////////
							if($el->hasChildNodes())
								foreach($el->childNodes as $cn)
									if($cn->nodeType==1)
										switch($cn->localName)
										{
										case "filter":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;
															
													$toRun = _create_function($cn,'$doc,$el',$functionText);
													$toRun($documentFrom,$el);
										break;
										case "check":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;


													$toRun = _create_function($cn,'$doc,$el',$functionText);
													$checkedFunctionReturnValue = $toRun($documentFrom,$el);
													
													$hasCheck = true;
										break;
										case "eachnode":
													if($cn->hasChildNodes())
														foreach($cn->childNodes as $child)
																if($child->nodeType == 7)
																		$functionText = $child->data;


													$withResults = _create_function($cn,'$el',$functionText);
										break;
										}

							if($hasCheck)
								{
								//daca functia returneaza o alta valoare in afara de false
								if($checkedFunctionReturnValue)
									//execut acelasi cod ca atunci cand nu avem un check pe acest element
									{
									//creez fragmentul de document in HTML-ul nostru
									$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
									
									$documentFrom->q($xpath);
									
									foreach($documentFrom->results as $result)
										{
										$newnode = $targetNodeInDoc->ownerDocument->importNode($result, TRUE);
										if(function_exists($withResults))
											{
											if($withResults($newnode))
												$docFragment->appendChild($newnode);
											}
											else $docFragment->appendChild($newnode);
										}
									
									//inlocuim elementul import cu fragmentul generat
									$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);					
									}
									else
										//in acest caz inlocuiesc element(import aici) cu nimic :D
										{
										$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
										//adaugam sursa nodului la fragment
										$docFragment->appendXML(" ");
										
										//inlocuim elementul import cu fragmentul generat
										$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);										
										}
								}
								else
									{
									//creez fragmentul de document in HTML-ul nostru
									$docFragment = $targetNodeInDoc->ownerDocument->createDocumentFragment();
									
									$documentFrom->q($xpath);

									
									foreach($documentFrom->results as $result)
										{
										$newnode = $targetNodeInDoc->ownerDocument->importNode($result, TRUE);
										if(function_exists($withResults))
											{
											if($withResults($newnode))
												$docFragment->appendChild($newnode);
											}
											else $docFragment->appendChild($newnode);
										}
									
									//inlocuim elementul import cu fragmentul generat
									$targetNodeInDoc->parentNode->replaceChild($docFragment,$targetNodeInDoc);
									}
							
							}
						}
						
function processMarkers(&$el)
						{
						$queryText = "";
						$filterText = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 3)
									$queryText = $child->nodeValue;
								

						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 3)
									$filterText = $child->nodeValue;
								
									
						$GLOBALS["translator"][$queryText] = $filterText;
						}
											
function processEvals(&$el)
						{
						$queryText = "";
						$filterText = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 3)
									$queryText = $child->nodeValue;
								

						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								{
								$checkForType = 7;
								
								if($child->nodeType == $checkForType)
									$filterText = $child->data;
								}
									
									
						$toEval = _create_function($el,"",$filterText);
						$GLOBALS["translator"][$queryText] = $toEval();
						}
						
function appInitDirective(&$el)
						{
						$functionText = "";
						$toReturnVarName = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								{
								if($child->nodeType == 7)
									$functionText = $child->data;
								else
									if($child->nodeType == 1)
										switch(strtolower($child->localName))
											{
											case "return":
												$toReturnVarName = $child->textContent;
											break;
											}
										
								}
						
						
						$toRun = _create_function($el,"",$functionText);

						if($toReturnVarName)
								$GLOBALS["APPINIT"][$toReturnVarName] = $toRun();
								else
								$toRun();							

							
						$el->parentNode->removeChild($el);
						}


function blockExecute(&$el)
						{
						$functionText = "";

						$functionText = $el->data;

						$pos = false;

						$elements = "";

						$sibling = false;

						for($pi=0; $pi < $el->parentNode->childNodes->length; $pi++) 
						        if($el->parentNode->childNodes->item($pi)->isSameNode($el))
									$pos = $pi+1;

						for($pi = $pos; $pi < $el->parentNode->childNodes->length; $pi++)
							if($el->parentNode->childNodes->item($pi)->nodeType != 7)
								$elements[]=$el->parentNode->childNodes->item($pi);
							else if($el->parentNode->childNodes->item($pi)->target == "php")
									{$sibling = $el->parentNode->childNodes->item($pi);break;}

						$functionText.=$sibling->data;

						$docFragment = $el->ownerDocument->createDocumentFragment();

						if(sizeof($elements)>0)
							foreach($elements as $Node)
								 	if(get_parent_class($Node)=="DOMNode")
								 		$docFragment->appendChild($Node->cloneNode(TRUE));	

						$toRun = _create_function($el,'&$el,&$fragment',$functionText);

						if(function_exists($toRun))
							{
								$result = $toRun($el->parentNode,$docFragment->cloneNode(TRUE));
								if(get_parent_class($result)=="DOMNode")
										{if($result !== $docFragment)
											$docFragment = $result;}
								else
										while($docFragment->hasChildNodes())
											$docFragment->removeChild($docFragment->firstChild); 
							}

						if(sizeof($elements)>0)
							foreach($elements as $Node)
								$el->parentNode->removeChild($Node);

						if(get_parent_class($sibling)=="DOMNode")
							$sibling->parentNode->removeChild($sibling);

						$el->parentNode->replaceChild($docFragment,$el);
						}

function userPHPCodeExecute(&$el)
						{
						$functionText = "";
						$toReturnVarName = "";
						
						if($el->nodeType == 7)
							$functionText = $el->data;
						else
							if($el->hasChildNodes())
								foreach($el->childNodes as $child)
									{
									$checkForType = 7;
									
									if($child->nodeType == $checkForType)
										$functionText = $child->data;
									else
										if($child->nodeType == 1)
											switch(strtolower($child->localName))
												{
												case "return":
													$toReturnVarName = &$child->textContent;
												break;
												}
											
									}
						
						$toRun = _create_function($el,'&$el',$functionText);
						if($toRun)
							{
							if($toReturnVarName)
								$GLOBALS[$toReturnVarName] = $toRun($el->parentNode);
								else
								$toRun($el->parentNode);							
							}
							
						$el->parentNode->removeChild($el);
						}
						
function runProcessingInstruction(&$el)
						{
						if($el->nodeType == 7)
							$functionText = $el->data;
						else
							foreach($el->childNodes as $child)
								if($child->nodeType == 7)
									$functionText = $child->data;
														
						$toRun = _create_function($el,'$el',$functionText);
						
						$docFragment = $el->ownerDocument->createDocumentFragment();
						//adaugam sursa nodului la fragment
						$docFragment->appendXML($toRun($el->parentNode));

						//inlocuim elementul import cu fragmentul generat
						$el->parentNode->replaceChild($docFragment,$el);
						}
						
function runCodeHere(&$el)
						{
						$functionText = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 7)
									$functionText = $child->data;
										
						
						
						$toRun = create_function('$el',$functionText);
						$toRun($el->parentNode);
						
						$el->parentNode->removeChild($el);
						}
						
function processReplacements(&$el)
							{
							$functionText = "";
							if(strtolower($el->localName) == "replace" && $el->nodeType ==1)
								{
								if($el->hasChildNodes())
									foreach($el->childNodes as $child)
										if($child->nodeType == 7)
											$functionText = $child->data;
											
								$toRun = create_function('$el',$functionText);
								
								$docFragment = $el->ownerDocument->createDocumentFragment();
										
													
								//adaugam sursa nodului la fragment
								$docFragment->appendXML($toRun($el->parentNode));
										
								//inlocuim elementul import cu fragmentul generat
								$el->parentNode->replaceChild($docFragment,$el);
								}
							}
						
						
//executa functia pentru cu parametru fiecare  element in parte
//accesul la parametru se face: $el = func_get_arg(0); sau $el 
function processXpath(&$el)
						{
						$functionText = "";
						$queryText = "";

						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 3)
									$queryText = $child->nodeValue;

						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 7)
										$functionText = $child->data;
						
						$GLOBALS["HTMLPARSER"]->q($queryText)->each(_create_function($el,'$el',$functionText));
						}
						
//executa functia pentru fiecare intregul nodelist apeland=o cu cu parametru de tip $GLOBALS["HTMLPARSER"];
//merge cu append, replace ... toate din clasa HTMLDoc
//accesul la parametru se face: $el = func_get_arg(0); sau $el
function processXpathNodeList(&$el)
						{
						$functionText = "";
						$queryText = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 3)
									$queryText = $child->nodeValue;

						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 7)
										$functionText = $child->data;
						
						$runMe = _create_function($el,'$el',$functionText);
						$runMe($GLOBALS["HTMLPARSER"]->q($queryText));
						}

/*
 * array(eticheta=>valoare, ....)
 * */
function processMatches(&$el)
						{
						$elementXPATH = "";
						$element = "";
						$elementC14n = "";
						$functionText = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								if($child->nodeType == 3)
									$elementXPATH = $child->nodeValue;

						//continutul functiei care trebuie sa returneze array pt strtr
						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 7)
									$functionText = $child->data;
						
						$runMe = _create_function($el,'$el',$functionText);
						$pattern = $runMe();
						
						//caut element din XPATH si il pun in var element
						$GLOBALS["HTMLPARSER"]->q($elementXPATH)->to($element);
						
						//luam sursa elem ca text
						foreach($element as $domel)
							$elementC14n.=$domel->C14N();
						
						
						foreach($pattern as $k=>$v)
								$elementC14n=preg_replace(AWS_ITERATOR_MATCH_PREFIX.$k.AWS_ITERATOR_MATCH_SUFFIX,$v,$elementC14n);
						
						//inlocuiesc sursa element din doc cu noua sursa
						$GLOBALS["HTMLPARSER"]->q($elementXPATH)->replace($elementC14n);
						}
						
						
/*
 * mai ales pentru interogare baze de date
 * array(0=> array(eticheta=>valoare, ....), 1=>=> array(eticheta=>valoare, ....),...)
 * */
function processMatchesIterator(&$el)
						{
						$elementXPATH = "";
						$element = "";
						$unit = "";
						$elementC14n = "";
						$functionText = "";
						$translator = "";
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								{
									if($child->nodeType == 3)
										$elementXPATH = $child->nodeValue;

									if($child->localName == "norecords")
										foreach($child->childNodes as $cnn)
											{
											$noRecordsFunctionText = $cnn->nodeValue;
															
											$toRun = _create_function($el,"",$noRecordsFunctionText);
												
											$noRecords = $GLOBALS["HTMLPARSER"]->doc->documentElement->ownerDocument->createDocumentFragment();
											//adaugam sursa nodului la fragment
											$noRecords->appendXML($toRun());
											}
								}


						//continutul functiei care trebuie sa returneze array pt strtr
						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 7)
									$functionText = $child->data;
						
						$runMe = _create_function($el,'$el',$functionText);
						$translator = $runMe();						
						
						//caut element din XPATH si il pun in var element
						$sursaQ4UMatchesIterator = $GLOBALS["HTMLPARSER"]->q($elementXPATH)->results->item(0); 
						$q4u = new awsHTML($sursaQ4UMatchesIterator);
						$q4u->q("descendant-or-self::*[@unit]")->to($unit);
						
						//luam sursa elem unit ca text
						$elementC14n=$unit->item(0)->C14N();
						
						$collection = "";
						
						if(sizeof($translator)>0)
							foreach($translator as $pattern)
								{
								$subj = $elementC14n;
								foreach($pattern as $k=>$v)
									$subj=preg_replace(AWS_ITERATOR_MATCH_PREFIX.$k.AWS_ITERATOR_MATCH_SUFFIX,$v,$subj);
									
								$collection.=$subj;
								}
								else
									if($noRecords instanceof DOMDocumentFragment)
										$unit->item(0)->parentNode->insertBefore($noRecords,$unit->item(0));
							
						//inlociuesc unit cu toata colectia de mai sus
						$q4u->replace($collection);	
						}
						
// array(0=> array(eticheta=>valoare, ....), 1=>=> array(eticheta=>valoare, ....),...)
function processDomIterator(&$el)
						{
						$elementXPATH = "";
						$unit = "";
						$functionText = "";
						$translator = "";
						$eachnamedreference = false;
						
						if($el->hasChildNodes())
							foreach($el->childNodes as $child)
								{
								if($child->nodeType == 3)
									$elementXPATH = $child->nodeValue;
								
								if($child->localName == "eachreference")
									foreach($child->childNodes as $cne)
										if($cne->nodeType == 7)
											$foreachreferenceFunction = _create_function($child,'&$el,$label,$value,$recordset',$cne->data);
											
								if($child->localName == "eachnamedreference")
									$eachnamedreference = $child;
											
								if($child->localName == "norecords")
									foreach($child->childNodes as $cnn)
										if($cnn->nodeType == 7)
											{
											$noRecordsFunctionText = $cnn->data;
															
											$toRun = _create_function($child,"",$noRecordsFunctionText);
												
											$noRecords = $GLOBALS["HTMLPARSER"]->doc->documentElement->ownerDocument->createDocumentFragment();
											//adaugam sursa nodului la fragment
											$noRecords->appendXML($toRun());
											}
								}

						//continutul functiei care trebuie sa returneze array pt strtr
						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 7)
									$functionText = $child->data;
						
						$runMe = _create_function($el->parentNode,'$el',$functionText);
						$translator = $runMe();						
						
						//caut element din XPATH si il pun in var element
						$sursaQ4UDinDocument = $GLOBALS["HTMLPARSER"]->q($elementXPATH)->results->item(0);
						$q4u = new awsHTML($sursaQ4UDinDocument);
						$q4u->q("descendant-or-self::*[@unit]")->to($unit);

						
						if(sizeof($translator)>0)
							foreach($translator as $pattern)
								{
								$df = $unit->item(0)->ownerDocument->createDocumentFragment();
								$df->appendXML($unit->item(0)->C14N());
								$unitClone =$df->firstChild;
								
								$unitClone->removeAttribute("unit");
								$unitClone->setAttribute("clone","TRUE");
								$unit->item(0)->parentNode->insertBefore($unitClone,$unit->item(0));

								$unitQ = new awsHTML($unitClone);
								
								foreach($pattern as $k=>$v)
									{
									$checkReplaceContent = 		_create_function($el,'&$el','if($el->hasAttribute("skip")) return false; else return true;');
									$checkReplaceContentNew = 	_create_function($el,'&$el','if($el->hasAttribute("modtype") && $el->getAttribute("modtype")=="replace") return true; else return false;');
									$checkAppend = 				_create_function($el,'&$el','if($el->hasAttribute("modtype") && $el->getAttribute("modtype")=="append") return true; else return false;');
									$checkPrepend = 			_create_function($el,'&$el','if($el->hasAttribute("modtype") && $el->getAttribute("modtype")=="prepend") return true; else return false;');
									
									if(!function_exists($foreachreferenceFunction))
											$unitQ->q('descendant-or-self::*[@reference="'.$k.'"]')->check($checkReplaceContent)->replaceContent($v)->check($checkAppend)->append($v)->check($checkPrepend)->prepend($v);
										else
											$unitQ->q('descendant-or-self::*[@reference="'.$k.'"]')->check($checkReplaceContent)->replaceContent($v)->check($checkAppend)->append($v)->check($checkPrepend)->prepend($v)->each($foreachreferenceFunction,$k,$v,$pattern);
									}
								
								foreach($pattern as $k=>$v)
									{
									//$eachnamedreference
									if($eachnamedreference)
										foreach($eachnamedreference->childNodes as $namedreferencenode)
											if($namedreferencenode->localName == $k)
												foreach($namedreferencenode->childNodes as $nrt)
													if($nrt->nodeType == 7)
														{
														$namedReferenceFunction = _create_function($namedreferencenode,'&$el,$label,$value,$recordset',$nrt->data);
														$unitQ->q('descendant-or-self::*[@reference="'.$k.'"]')->each($namedReferenceFunction,$k,$v,$pattern);
														}
									}
								}
								else
									if($noRecords instanceof DOMDocumentFragment)
										$unit->item(0)->parentNode->insertBefore($noRecords,$unit->item(0));
								

						$q4u->q('descendant::*[@clone="TRUE"]')->removeAttr("clone");
						
						//unit remove
						$unit->item(0)->parentNode->removeChild($unit->item(0));
						}
						

function langSysMessages(&$el)
					{
					$targetName = '//*[@id="'.$el->getAttribute("id").'"]';
					//element folosit pentru selectarea copiilor dupa xpath dat in acest atribut; poate fi folosit pentru intreg tree-ul cu xpath
					$childPath = "";
					if($el->hasAttribute("target"))
						$targetName = $el->getAttribute("target");
					
					$source = "";
					foreach($el->childNodes as $child)
							$source.=$child->C14N();
						
					$GLOBALS["HTMLPARSER"]->q($targetName)->replaceContent($source);
					}
					
function getFilter(&$el)
			{
			$tmpFilter = array();
			
			foreach($el->childNodes as $child)
					if($child->nodeType == 1)
						switch(strtolower($child->localName))
							{
							case "alias":
									foreach($child->childNodes as $textNode)
										if($textNode->nodeType == 3)
											$tmpFilter[strtolower($child->localName)] = $textNode->textContent;
							break;
							case "check":
									foreach($child->childNodes as $textNode)
										if($textNode->nodeType == 7)
											$tmpFilter[strtolower($child->localName)] = _create_function($child,'',$textNode->data);
							break;
							case "xpath":
									foreach($child->childNodes as $textNode)
										if($textNode->nodeType == 3)
											$tmpFilter[strtolower($child->localName)] = $textNode->textContent;
							break;
							case "callback":
									foreach($child->childNodes as $textNode)
										if($textNode->nodeType == 7)
											$tmpFilter[strtolower($child->localName)] = _create_function($child,'&$el',$textNode->data);
							break;
							}
				
				
			$GLOBALS["userParsers"][] = $tmpFilter;
			}


function copy_Head_and_Content(&$el,&$target)
							{
							$h = $target->q("//*[local-name()='header']")->results->item(0);
							$c = $target->q("//*[local-name()='content']")->results->item(0);
														
							if($el->localName == "header")
								{
								if($el->hasAttributes)
									foreach($el->attributes as $attr)
										$h->setAttribute($attr->name,$attr->value);
										
								if($el->hasChildNodes())
									foreach($el->childNodes as $cn)
										$h->appendChild($target->doc->importNode($cn,TRUE));
								}
								
							if($el->localName == "content")
								{
								if($el->hasAttributes)
									foreach($el->attributes as $attr)
										$c->setAttribute($attr->name,$attr->value);
										
								if($el->hasChildNodes())
									foreach($el->childNodes as $cn)
										$c->appendChild($target->doc->importNode($cn,TRUE));
								}
								
							if($el->parentNode->hasAttributes())
								foreach($el->parentNode->attributes as $pattr)
									$target->doc->documentElement->setAttribute($pattr->name,$pattr->value);
							}

function doXSLToHeaderContentTransform(&$el)
					{
					$insrn = "<client><header/><content/></client>";
					$temp = new awsXML($insrn);
					
					$GLOBALS["TEMPLATE"]->q("//*[local-name()='app']/*[local-name()='client']/*")->each("copy_Head_and_Content",$temp);

					$GLOBALS["HTMLPARSER"] = $temp;
					}

function execDirectiveFromFilter(&$el)
					{
						if($el->parentNode->hasChildNodes())
							foreach($el->parentNode->childNodes as $child)
								if($child->nodeType == 3)
										$queryText = $child->nodeValue;
										
					$GLOBALS["HTMLPARSER"]->q($queryText);
					
					foreach($GLOBALS["HTMLPARSER"]->results as $result)
						switch(strtolower($el->localName))
						{
							case "import":
								doImport($el,$result);
								break;

							case "advancedimport":
								doAdvancedImport($el,$result);
								break;
						}
					}
						
function obFilter($buffer)
					{
					if(gettype($GLOBALS["translator"]) == "array")
						$buffer = strtr($buffer,$GLOBALS["translator"]);
					if($GLOBALS["TEMPLATE"]->doc->documentElement->getAttribute('outputdisabled') != "TRUE")
						return $buffer;
					}

//app Name for lang and config
$appName					= "";

//user defined parsers
$userParsers = array();

//BROWSER DETECTION
$GLOBALS["BROWSER"] = new Browser;

//OUTPUT TRANSLATOR ARRAY
$GLOBALS["translator"] ="";

///////////////////////////////////
///////////////CONFIG//////////////
///////////////////////////////////
if($_GET["config"])
		$awsappconfigurationFile = file_get_contents($_GET["config"]);
else
	if(file_exists("config.xml"))
		$awsappconfigurationFile = file_get_contents("config.xml");

if($awsappconfigurationFile)
	$GLOBALS["CONFIG"] 	= new awsXML($awsappconfigurationFile);

///////////////////////////////////
///////////////LANG////////////////
///////////////////////////////////

if($_GET["langSource"] && file_exists($_GET["langSource"]))
	$awsapplanguageFile = file_get_contents($_GET["langSource"]);
else
if(file_exists("lang.xml"))
		$awsapplanguageFile = file_get_contents("lang.xml");

if($awsapplanguageFile)
	$GLOBALS["LANG"] 	= new awsXML($awsapplanguageFile);	
		

			

///////////////////////////////////////
///////////////XML TEMPLATE////////////
///////////////////////////////////////
if($_GET["use"])
	{
	if(!file_exists($_GET["use"]) || !is_file($_GET["use"]))
		$awsappxmltemplateFile = file_get_contents(AWS_ERROR_404);
		else 
		$awsappxmltemplateFile = file_get_contents($_GET["use"]);
	}
else
	$awsappxmltemplateFile = file_get_contents(AWS_HOME);
	
$GLOBALS["TEMPLATE"] 	= new awsXML($awsappxmltemplateFile);
	
///////////////////////////////////
////////////USER PARSERS///////////
///////////////////////////////////

if(file_exists("parsers.xml"))
	{
	$awsappglobalparsersFile = file_get_contents("parsers.xml");
	$GLOBALS["FILTERS"] 	= new awsXML($awsappglobalparsersFile);
	}


///////////////////////////////////////
/////////////////HTML//////////////////
///////////////////////////////////////

$GLOBALS["HTMLPARSER"] 		= FALSE;

$parsers = array(
				array(
						"alias"		=> "remote Template",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='remotetemplate']",
						"callback"	=> "loadRemoteTemplate"
					 ),
				array(
						"alias"		=> "use",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='use']",
						"callback"	=> "doFilterImport"
					 ),
				array(
						"alias"		=> "init",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='init']",
						"callback"	=> "appInitDirective"
					 ),
				array(
						"alias"		=> "name",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='name']",
						"callback"	=> "getAppName"
					 ),
				array(
						"alias"		=> "template:/app/client|header",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "/",
						"callback"	=> "doXSLToHeaderContentTransform"
					 ),
				array(
						"alias"		=> "case",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='case']",
						"callback"	=> "processCases"
					 ),
				array(
						"alias"		=> "import",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='import']",
						"callback"	=> "doImport"
					 ),
				array(
						"alias"		=> "importfilter",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='dom']/*[local-name()='filter']/*[local-name()='import']",
						"callback"	=> "execDirectiveFromFilter"
					 ),
				array(
						"alias"		=> "advancedimport",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='advancedimport']",
						"callback"	=> "doAdvancedImport"
					 ),
				array(
						"alias"		=> "advancedimportfilter",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='dom']/*[local-name()='filter']/*[local-name()='advancedimport']",
						"callback"	=> "execDirectiveFromFilter"
					 ),
				array(
						"alias"		=> "#pi php",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//processing-instruction('php')[position() mod 2 != 0]",
						"callback"	=> "blockExecute"
					 ),
				array(
						"alias"		=> "exec",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='exec']|//processing-instruction('exec')",
						"callback"	=> "userPHPCodeExecute"
					 ),
				array(
						"alias"		=> "langsys",
						"check"		=> _create_function($NONE,'','return $_SESSION["lang"];'),
						"target"	=> "LANG",
						"xpath"		=> "//*[local-name()='lang']/*[local-name()='".$_SESSION["lang"]."']/*[local-name()='".$GLOBALS['appName']."']/*[local-name()='sys']/*",
						"callback"	=> "langSysMessages"
					 ),
				array(
						"alias"		=> "langui",
						"check"		=> _create_function($NONE,'','return $_SESSION["lang"];'),
						"target"	=> "LANG",
						"xpath"		=> "//*[local-name()='lang']/*[local-name()='".$_SESSION["lang"]."']/*[local-name()='".$GLOBALS['appName']."']/*[local-name()='ui']/*",
						"callback"	=> "langSysMessages"
					 ),
				array(
						"alias"		=> "langerr",
						"check"		=> _create_function($NONE,'','return $_SESSION["lang"];'),
						"target"	=> "LANG",
						"xpath"		=> "//*[local-name()='lang']/*[local-name()='".$_SESSION["lang"]."']/*[local-name()='".$GLOBALS['appName']."']/*[local-name()='err']/*",
						"callback"	=> "langSysMessages"
					 ),
				array(
						"alias"		=> "xpath",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='dom']/*[local-name()='filter']/*[local-name()='xpath']",
						"callback"	=> "processXpath"
					 ),
				array(
						"alias"		=> "nodelist",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='dom']/*[local-name()='filter']/*[local-name()='nodelist']",
						"callback"	=> "processXpathNodeList"
					 ),
				array(
						"alias"		=> "domiterator",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='dom']/*[local-name()='filter']/*[local-name()='domiterator']",
						"callback"	=> "processDomIterator"
					 ),
				array(
						"alias"		=> "match",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='ob']/*[local-name()='filter']/*[local-name()='match']",
						"callback"	=> "processMatches"
					 ),
				array(
						"alias"		=> "matchiterator",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='ob']/*[local-name()='filter']/*[local-name()='matchiterator']",
						"callback"	=> "processMatchesIterator"
					 ),
				array(
						"alias"		=> "marker",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='ob']/*[local-name()='filter']/*[local-name()='marker']",
						"callback"	=> "processMarkers"
					 ),
				array(
						"alias"		=> "eval",
						"check"		=> "",
						"target"	=> "TEMPLATE",
						"xpath"		=> "//*[local-name()='app']/*[local-name()='filters']/*[local-name()='ob']/*[local-name()='filter']/*[local-name()='eval']",
						"callback"	=> "processEvals"
					 ),
				array(
						"alias"		=> "replace",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='replace']",
						"callback"	=> "processReplacements"
					 ),
				array(
						"alias"		=> "runhere",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//*[local-name()='runhere']",
						"callback"	=> "runCodeHere"
					 ),
				array(
						"alias"		=> "#pi/aws",
						"check"		=> "",
						"target"	=> "HTMLPARSER",
						"xpath"		=> "//processing-instruction('aws')",
						"callback"	=> "runProcessingInstruction"
					 ),
				);
				


foreach($parsers as $parser)
	{
	$check = TRUE;
	
	if(function_exists($parser["check"]))
		$check = $parser["check"]();
		
	if($check)
		$$parser["target"]->q($parser["xpath"])->each($parser["callback"]);
	}

					//////////////////////////////////////
					////GET GLOBAL SCOPE USER DIRECTIVES//
					//////////////////////////////////////

if($GLOBALS["FILTERS"] instanceOf awsXML)
	$GLOBALS["FILTERS"]->q("//*[local-name()='parsers']/*[local-name()='item']")->each("getFilter");

					//////////////////////////////////////
					////GET APP RELATED USER DIRECTIVES///
					//////////////////////////////////////
	
$GLOBALS["TEMPLATE"]->q("//*[local-name()='app']/*[local-name()='parsers']/*[local-name()='item']")->each("getFilter");

					//PARSE USER DIRECTIVES//
foreach($userParsers as $parser)
	{
	$check = TRUE;
	
	if(function_exists($parser["check"]))
		$check = $parser["check"]();
		
		
	if($check)
		$GLOBALS["HTMLPARSER"]->q($parser["xpath"])->each($parser["callback"]);
	}

					//////////////////////////////////////
					////////////OUTPUT BUFERING///////////
					//////////////////////////////////////

ob_start("obFilter");

//using a custom xstyle for output
$outputxsl = $GLOBALS["TEMPLATE"]->doc->documentElement->getAttribute('outputxsl');

if($outputxsl)
		echo $GLOBALS["HTMLPARSER"]->content($outputxsl);
	else
		echo $GLOBALS["HTMLPARSER"]->content();

ob_end_flush();


// log output from _create_function if enabled
if(AWS_DEBUG_ALL_LAMBDA)
		{file_put_contents(AWS_DEBUG_ALL_LAMBDA_FILENAME.".gz",$errLogMessage,FILE_APPEND);
		file_put_contents("compress.zlib://log/GLOBAL.gz","\n\n\n\nSESSION ".session_id()."\n\n".$errLogMessage,FILE_APPEND);}
		
?>
