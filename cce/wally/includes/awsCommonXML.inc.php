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

//0.10-2010-11-03: dirty output fixes
//0.9: filter(xpath) - query on a clone of $this;
//0.9: check , filter(function) - works, filter(xpath) - works
//0.8: each - now support multiple arguments; these will be transmitted to the function; first arg will be always $el



define('AWS_COMMON_XML_INCLUDED', TRUE);

interface awsXMLInterface
	{
	public function content();
	}
	
	
	
interface awsHTMLInterface
	{
	public function content();
	public function body();
	public function head();
	public function getScripts();
	public function getLinks();
	public function getStyles();
	public function bodyContent();
	public function headContent();
	public function documentContent();
	public function addStyle( $url, $media='all', $user = false);
	public function addScript ( $url, $run = false );
	public function addMeta ( $name, $content );
	public function setDescription ( $dec );
	public function setKeywords ( $keywords );
	}


abstract class awsCommonXML
	
{

const	awsCommonXMLVersion = "0.9";


public	$doc;
public	$results;
public 	$current;
private	$check;
private	$filter;

public	$transformer;

							
final public function cssq($query = ".",$context = FALSE)
						{
							try
								{
								if(!$context)
									$this->results = $this->xpath->query($this->transformer->transform($query),$this->context);
									else
									$this->results = $this->xpath->query($this->transformer->transform($query),$context);
																	
								
							    	return $this;									
								}
								catch (Exception $e)
										{
										echo "\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n";
										}
						}
						
final public function csse($query = ".",$context = FALSE)
						{
							try
								{
								if(!$context)
									$this->results = $this->xpath->query($this->transformer->transform($query),$this->context);
									else
									$this->results = $this->xpath->query($this->transformer->transform($query),$context);
																	
								
							    	return $this;									
								}
								catch (Exception $e)
										{
										echo "\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n";
										}
						}


final public function q($query = ".",$context = FALSE)
						{
						try
								{
								if(!$context)
									$this->results = $this->xpath->query($query,$this->context);
									else
									$this->results = $this->xpath->query($query,$context);
																	
								
							    	return $this;									
								}
								catch (Exception $e)
										{
										echo "\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n";
										}
						}
						
						
						
final public function e($query = ".",$context = FALSE)
						{
							try
								{
								if(!$context)
									$this->results = $this->xpath->evaluate($query,$this->context);
									else
									$this->results = $this->xpath->evaluate($query,$context);
																	
								
							    	return $this;									
								}
								catch (Exception $e)
										{
										echo "\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n";
										}
						}

						
final public function check($toCheck)
							{
							if(function_exists($toCheck))
									$this->check = $toCheck;
								else
									$this->check = FALSE;
							
							return $this;
							}
							
final public function filter($filter)
							{
							if($filter)
									$this->filter = $filter;
								else
									$this->filter = FALSE;
							
							return $this;
							}
							
function __clone()
			{
			$this->results = "";
			$this->check = FALSE;
			$this->filter = FALSE;
			}
						
final public function each($callback)
							{
							if(func_num_args()>0)
									{
									$params = func_get_args();
									
									if(function_exists($callback))
										{
										if(!$this->results)
											$this->q(".");
										
										if(!$this->filter)
											foreach ($this->results as $result)
													{
													$params[0] = &$result;
													@call_user_func_array($callback,&$params);
													}
										else
											{
											if(!function_exists($this->filter))
												{//pt selector												
												foreach ($this->results as $result)
													{
														$p = clone $this;
														
														$p->q($this->filter,$result);
														foreach($p->results as $nr)
															{
															$params[0] = &$nr;
															@call_user_func_array($callback,&$params);
															}
													}
												}
												else
													{//pt functie
													foreach ($this->results as $result)
														if(call_user_func($this->filter,$result))
															{
															$params[0] = &$result;
															@call_user_func_array($callback,&$params);
															}
													}
											}
										}			
									}
							return $this;
							}
							
							
final public function _each($callback)
							{
							// 5.3 de verificat
							if(function_exists($callback))
								{
								while($this->iterate())
									//$callback($this->current);
									call_user_func($callback,$this->current);
								}
							return $this;
							}


final public function getRootElement()
	{
		return $this->doc->documentElement;
	}


	
	
final public function attr($attrName,$attrValue)
	{
	
		if(func_num_args()>0)
			{
			@$doReplace = create_function('&$el,$check,$attrName,$attrValue','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {if($el->nodeType == 1) $el->setAttribute($attrName,$attrValue);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$attrName,$attrValue);
				else
				$this->each($doReplace,FALSE,$attrName,$attrValue);
			}
	return $this;
	}
	
final public function removeAttr()
	{
	
		if(func_num_args()>0)
			{
			$attrName = func_get_arg(0);
			@$doReplace = create_function('&$el,$check,$attrName','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {if($el->nodeType == 1) if($el->hasAttribute($attrName)) $el->removeAttribute($attrName);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$attrName);
				else
				$this->each($doReplace,FALSE,$attrName);
			}
	return $this;
	}

final public function append()
	{
	
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);
			@$doReplace = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);$el->appendChild($docFragment);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$docFragmentSource);
				else
				$this->each($doReplace,FALSE,$docFragmentSource);
			}
	return $this;
	}
	
final public function prepend()
	{
	
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);
			@$doReplace = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);if($el->hasChildNodes()) $el->insertBefore($docFragment,$el->firstChild); else $el->appendChild($docFragment);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$docFragmentSource);
				else
				$this->each($doReplace,FALSE,$docFragmentSource);
			}
	return $this;
	}
	
final public function before()
	{
	
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);
			@$doReplace = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);$el->parentNode->insertBefore($docFragment,$el);}');			
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$docFragmentSource);
				else
				$this->each($doReplace,FALSE,$docFragmentSource);
			}
	return $this;
	}
	
final public function after()
	{
	
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);

			@$doReplace = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);$el->parentNode->insertBefore($docFragment,$el->nextSibling);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$docFragmentSource);
				else
				$this->each($doReplace,FALSE,$docFragmentSource);
			}
	return $this;
	}
	
final public function replace()
	{
	
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);
			
			@$doReplace = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);$el->parentNode->replaceChild($docFragment,$el);}');
			
			if(function_exists($this->check))
				$this->each($doReplace,$this->check,$docFragmentSource);
				else
				$this->each($doReplace,FALSE,$docFragmentSource);
			}
	return $this;
	}
	
	
final public function replaceContent()
	{
		if(func_num_args()==1)
			{
			$docFragmentSource = func_get_arg(0);
			
			@$doReplaceContent = create_function('&$el,$check,$docFragmentSource','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {while($el->hasChildNodes()) $el->removeChild($el->firstChild); $docFragment = $el->ownerDocument->createDocumentFragment();$docFragment->appendXML($docFragmentSource);$el->appendChild($docFragment);}');
			
			if(function_exists($this->check))
				$this->each($doReplaceContent,$this->check,$docFragmentSource);
				else
				$this->each($doReplaceContent,FALSE,$docFragmentSource);
			}
			
	return $this;
	}
	
final public function removeChilds()
			{
			@$doRemove = create_function('&$el,$check','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) while($el->hasChildNodes()) $el->removeChild($el->firstChild);');
			
			if(function_exists($this->check))
					$this->each($doRemove,$this->check);
				else
					$this->each($doRemove,TRUE);
			
			return $this;
			}
	
final public function text()
	{
	
		if(func_num_args()==1)
			{
			$textContent = func_get_arg(0);
			@$doReplaceContent = create_function('&$el,$check,$textContent','if(!$check || ($check && function_exists($check) && call_user_func($check,&$el))) {$done = false;if($el->nodeType == XML_ELEMENT_NODE){if($el->hasChildNodes()) foreach($el->childNodes as $child) if($child->nodeType == XML_TEXT_NODE) {$child->replaceData(0,strlen($child->wholeText),$textContent);$done = true;} if(!$done) $el->appendChild($el->ownerDocument->createTextNode($textContent));$done = false;}}');
			
			if(function_exists($this->check))
				$this->each($doReplaceContent,$this->check,$textContent);
				else
				$this->each($doReplaceContent,FALSE,$textContent);
						
			return $this;
			}
			else
				{
				$toReturn = "";
				foreach ($this->results as $result) 
					{
					//elemente
					if($result->nodeType == XML_ELEMENT_NODE)
						$toReturn.=$result->textContent;
					}
				return $toReturn;
				}
	
	}

	
final public function resultsAsSource()
	{
				$toReturn = "";
				foreach ($this->results as $result) 
					{
					//elemente
					if($result->nodeType == XML_ELEMENT_NODE)
						$toReturn.=$result->C14N();
					}
				return $toReturn;
	
	}
	
final public function allResultsAsSource()
	{
				$toReturn = "";
				
				foreach ($this->results as $result) 
						$toReturn.=$result->C14N();
						
				return $toReturn;
	
	}

	
final public function resultsAsDocumentFragment()
	{
				$docFragment = $this->doc->createDocumentFragment();

				foreach ($this->results as $result) 
					$docFragment->appendChild($result);
					
				return $docFragment;
	
	}

	

final public function to(&$destination)
	{
		$destination = $this->results;
		return $this;
	}
	
public function copyElement( &$domElem, &$target, $newName,$skipChilds = false, $skipAttributes = false)
        {
            //numele elementului
            //daca nu este dat newName luam numele elementului
            if(!$newName)
            	$name = $domElem->nodeName;
            	else 
            		$name = $newName;
	        
            //creez element
            $element = $this->doc->createElement($newName, ' ');

            //copiere atribute
            if(!$skipAttributes)
            	if($domElem->hasAttributes())
            		foreach($domElem->attributes as $attr)
            			$element->setAttribute($attr->name,$attr->value);
           //copiere copii
           if(!$skipChilds)
           	if($domElem->hasChildNodes())
           		foreach($domElem->childNodes as $child)
           			{
           			$newChild = $this->doc->importNode($child,true);
           			$element->appendChild($newChild);
           			}
           	//adaug elementul la target
            $target->appendChild( $element );
        }

 final public function first()
		 {
			if(!$this->results)
					$this->q("//*");
			if(get_class($this->results) == "DOMNodeList")
				{ return $this->results->item(0);}
				else return false;
				
		
		 }
		 
 final public function last()
		 {
			if(!$this->results)
					$this->q("//*");
			if(get_class($this->results) == "DOMNodeList")
		 		{return $this->results->item($this->results->length-1);}
				else return false;
		 }
	
final public function iterate()
		{
		$returnNext = false;
		$toReturn = false;
		
		if(!$this->current)
			{
			$toReturn = $this->first();
			$this->current=$this->first();
			}
			
		else
				{
				foreach($this->results as $result)
					if(!$returnNext)
						{
						if($result->isSameNode($this->current))
								$returnNext = true;
						}
						else {$this->current = $result; $toReturn = $this->current;break;}
				}
		
		return $toReturn;
		}
		
}
?>