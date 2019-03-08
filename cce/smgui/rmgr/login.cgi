# 
#!/usr/local/bin/perl 
####################################
#filename: login.cgi
#check username and password
# Anil Sharma
####################################
use strict;
use CGI 'param','header';

open PWFILE, "../data/userFile" or die "cannot open userFile $!" ;
my @pwfile=<PWFILE>;
close PWFILE;

my $user=lc(param("Username"));
my $pass=param("Password");

for (@pwfile) {
    my $encrypsw=crypt($pass,"00");
    if (( m/^$user:.*:faculty:.*:$encrypsw$/ ) || ( m/^$user:.*:staff:.*:$encrypsw$/ )) {
        print "Location: ../html/aatatool.html\n\n";
	exit;
    }
}
print "Location: ../reattemptlogin.html\n\n";
