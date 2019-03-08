#!/usr/local/bin/perl

use strict;
use CGI 'param','header';

print header( {-type=>'text/plain', -expires=>'now'} );

my $user = lc(param("user"));
my $pass = param("pass");

#debug
print "user: ", $user, "\npassword: ", $pass, "\n";

open (PWFILE,"passwords") || die "can't open passwords file";

while ( <PWFILE> )
{
	#if ( /some regular expression here/ )
	#{
	#	print homepage();
	#	exit;
	#}
}

print "Incorrect username or password.\n";

