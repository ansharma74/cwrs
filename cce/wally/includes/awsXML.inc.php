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

define('AWSXML_INCLUDED', TRUE);


class awsXML
		extends awsCommonXML
			implements awsXMLInterface
	
{
const	iof = 'awsXML';
const	version = 0.10;
const	releaseDate = "2010-10-29";
const	credentials = "";

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
										$this->doc->loadXML($context);
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
																	if(get_class($context) =="awsXML")
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
										$this->context = &$this->doc;
										}
								
								
								$this->xpath = new DOMXPath($this->doc);
								}
							catch (Exception $e)
										{
										file_put_contents("awsXMLError.log","\n\t Exception '".__CLASS__ ."' with message '".$e->getMessage()."' in ".$e->getFile().":".$e->getLine()."\nStack trace:\n".$e->getTraceAsString()."\n----------------------------\n");
										}
						}
							

final public function content($xstyle = 'xsl/namespacefix-xmldoc.xsl')
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
	return $this->doc->saveXML();
	}
						
final public function contentAs($outputType)
	{
	if(!$outputType) $outputType = "xml";
	if(AWS_HTML_XSL_NAMESPACE_FIX)
		{
		// Load the XML source
		$xml = $this->doc;
		
		$xsl = new DOMDocument("1.0",$this->encoding);
		$this->doc->encoding = $this->encoding;
		$xsl->load('xsl/namespacefix-'.$outputType.'doc.xsl');
		
		// Configure the transformer
		$proc = new XSLTProcessor;
		// attach the xsl rules
		$proc->importStyleSheet($xsl);
		
		return $proc->transformToXML($xml);
		
		}
		
	else
	return $this->doc->saveXML();
	}
	
}


?>