#
#!/usr/local/bin/perl
###################################################################
# File:          lab_confirm.cgi
# Description:   Perl cgi script to process lab reserving and lab 
#                schdule displaying
#
# Student:       Anil Sharma
####################################################################

use strict;
use CGI ':standard';

#declare variables

my $Group=param("Group");
my $Lab=param("Lab");

print qq|
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<meta name="Generator" content="Microsoft FrontPage 5.0">
<title>Aristo ATA</title>
</head>
<table border="0" cellpadding="0" cellspacing="0" height="100%" width="100%">
<tbody>
<tr align="left" valign="top">
<td colspan="3" bgcolor="#CC0099" height="20%" width="100%" align="left">
<p><b><font size="5">Aristo Automated Technical Assistant (ATA)</font></b></p></td>
</tr>
<tr align="left" valign="top">
<td colspan="3" bgcolor="#CCCCFF" height="60%" width="100%" align="middle" valign="middle">
<p><b><font size="4">The Group:$Group in Lab:$Lab Reserved Successfully. <br/>Thank you for using ATA</font></b></p></td>
</tr>
<tr align="left" valign="top">
<td colspan="3" bgcolor="#CCCCFF" height="20%" align="middle"><a href="../html/aatatool.html"><input value="Back to Aristo ATA main page" type="button" height="50"></td>
</tr>
</tbody>
</table>
</html>
|;
