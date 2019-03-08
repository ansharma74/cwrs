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

//0.8: namespace fix when saving (::content()) xml  file with XSLT transformer
//0.8: context set up after instance type; further queries made on this context if no other specified
//0.8: using gets the document from dom object , instead dumping and using the source; this way it will affect the existing object, without creating another document
//0.8: cssq and csse - css query and css evaluate; transformer provided by Zend_Dom_Query_Css2Xpath/ZendFramework-1.10.8


define('AWSHTML_INCLUDED', TRUE);

class awsHTML
		extends awsCommonXML
			implements awsHTMLInterface
{ 
const	iof = 'awsHTML';
const	version = 0.12;
const	releaseDate = "2010-11-03";
const	credentials = "";

public  $head;
public  $body;
public	$context;
public	$encoding;


public function __construct(&$context,$encoding = 'UTF-8')
						{
						$this->transformer = new Zend_Dom_Query_Css2Xpath;
						$this->encoding = $encoding;
							try
								{
								if(func_num_args()==1)
									{
									if(gettype($context) =="string")
													{
													$this->doc = new DOMDocument("1.0",$this->encoding);
													$this->doc->encoding = $this->encoding;
													@$this->doc->loadHTML($context);
													$this->context = &$this->doc;
													}
													
									if(gettype($context)=="object")
													{
														if(get_class($context) =="DOMDocument")
															{
															$this->doc = $context;
															$this->context = &$this->doc;
															}
														else
															if(get_class($context) =="DOMNode" || get_class($context) =="DOMElement")
																{
																$this->doc = $context->ownerDocument;
																$this->context = $context;
																}
																else
																	if(get_class($context) =="awsHTML")
																		{
																		$this->doc = $context->doc;
																		$this->context = &$this->doc;
																		}
													}
									}
									else
										{
										$this->doc = new DOMDocument("1.0",$this->encoding);
										$this->doc->encoding = $this->encoding;
										$this->head = $this->doc->createElement( 'head', ' ' );
	            						$this->body = $this->doc->createElement( 'body', ' ' );
							            $this->doc->documentElement->appendChild( $this->head );
							            $this->doc->documentElement->appendChild( $this->body );
										$this->context = &$this->doc;
										}
								
								
								$this->xpath = new DOMXPath($this->doc);
								$this->head = $this->head();
								$this->body = $this->body();
								}
							catch (Exception $e)
										{
										file_put_contents("awsHTMLError.log","\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n");
										}
						}
							



final public function content($xstyle = 'xsl/namespacefix-htmldoc.xsl')
	{
	if(AWS_HTML_XSL_NAMESPACE_FIX)
		{
		// Load the XML source
		$xml = $this->doc;
		
		$xsl = new DOMDocument("1.0",$this->encoding);
		$this->doc->encoding = $this->encoding;
		$xsl->load($xstyle);
		
		// Configure the transformer
		$proc = new XSLTProcessor;
		// attach the xsl rules
		$proc->importStyleSheet($xsl);
		
		return $proc->transformToXML($xml);
		
		}
		
	else
		return $this->doc->saveHTML();
	}
						


	
final public function body()
	{
		return $this->doc->getElementsByTagName('body')->item(0);
	}
	
final public function head()
	{
		return $this->doc->getElementsByTagName('head')->item(0);
	}
	
final public function getScripts()
	{
		return $this->doc->getElementsByTagName('script');
	}
	
final public function getLinks()
	{
		return $this->doc->getElementsByTagName('link');
	}
	
final public function getStyles()
	{
		return $this->doc->getElementsByTagName('style');
	}

final public function bodyContent()
	{
		return $this->body()->C14N();
	}
	
final public function headContent()
	{
		return $this->head()->C14N();
	}
	
final public function documentContent()
	{
		return $this->content();
	}


public function addStyle( $url, $media='all', $user = false)
        {
            
            if(!$user)
	            {
            	$element = $this->doc->createElement('link',' ');
            	$element->setAttribute('type', 'text/css');
            	$element->setAttribute('rel', 'stylesheet');
	            $element->setAttribute('href', $url);
	            }
            	
            	else
	            	{
		            $element = $this->doc->createElement( 'style',' ' );
					$element->appendChild($element->ownerDocument->createTextNode($url));
	            	}
            	
            $element->setAttribute( 'media', $media );
            $this->head->appendChild( $element );
        }
       

        
public function addScript ( $url, $run = false )
        {
            $element = $this->doc->createElement( 'script', ' ' );
            $element->setAttribute( 'type', 'text/javascript' );
            
            if(!$run)
            	$element->setAttribute( 'src', $url );
            	else
            	$element->appendChild($element->ownerDocument->createTextNode($run));
            		
            $this->head->appendChild( $element );
        }
       
       
public function addMeta ( $name, $content )
        {
            $element = $this->doc->createElement( 'meta',' ');
            $element->setAttribute( 'name', $name );
            $element->setAttribute( 'content', $content );
            $this->head->appendChild( $element );
        }
       
       
public function setDescription ( $dec )
        {
            $this->addMeta( 'description', $dec );
        }
       
       
public function setKeywords ( $keywords )
        {
            $this->addMeta( 'keywords', $keywords );
        }

}


?>