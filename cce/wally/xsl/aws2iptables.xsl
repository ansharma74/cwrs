<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:template match="iptables-rules">
    <xsl:copy-of select=".">
    </xsl:copy-of>
  </xsl:template>
  
<xsl:output method="xml" indent="yes" omit-xml-declaration="yes" />

</xsl:stylesheet>
