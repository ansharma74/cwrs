<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


<xsl:variable name="smallcase" select="'abcdefghijklmnopqrstuvwxyz'" />
<xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />

  <xsl:template match="*">
    <xsl:element name="{local-name()}">
      <!-- go process attributes and children -->
      <xsl:apply-templates select="@*|node()"/>
    </xsl:element>
  </xsl:template>

  <xsl:template name="actions" match="actions">
	  <xsl:element name="{local-name()}">
	  	<xsl:for-each select="./*">
		  <xsl:choose>
		  
			  <xsl:when test="name()='call'">
			  	<xsl:copy-of select=".">
			  	</xsl:copy-of>
			  </xsl:when>
		  
			  <xsl:otherwise>				
				<xsl:element name="{translate(name(), $smallcase, $uppercase)}">
				  	<xsl:for-each select="./*">
				  		<xsl:copy-of select="."></xsl:copy-of>
				  	</xsl:for-each>
				</xsl:element>
			  </xsl:otherwise>
			  
		  </xsl:choose>
	  	</xsl:for-each>
	  </xsl:element>
  </xsl:template>
  
  <xsl:template name="allattributes" match="@*">
    <xsl:attribute name="{local-name()}">
      <xsl:value-of select="."/>
    </xsl:attribute>
  </xsl:template>
  
<xsl:output method="xml"/>

</xsl:stylesheet>


