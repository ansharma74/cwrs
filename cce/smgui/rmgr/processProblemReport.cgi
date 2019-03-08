#
#!/usr/local/bin/perl
###########################################################################
# Program:       processProblemReport.cgi
# Version:       1.0 Feb 28 2005
# Description:   Perl cgi script to take take a new problem, assign tech 
#                assistant and mail information to user.
# Author:        Anil Sharma
#                sharma_anil@yahoo.com
# Comments:      
###########################################################################

use strict;
use CGI 'param','header';

my $name=param("name");
my $Username=param("Username");
my $Password=param("Password");
my $department=param("Dept");
my $email=param("email");
my $phone=param("phone");
my $machine=param("machine");
my $category=param("category");
my $wellknownproblems=param("wellknownproblems");
my $description=param("description");
my $location=param("location");
my $time=param("time");

#Debug Block
#print header({-type=>'text/plain', -expire=>'now'});
#print "Info received $phone $machine $category $department\n";

#Check username/password
open PWFILE, "../data/userFile" or die "cannot open userFile $!" ;
my @pwfile=<PWFILE>;
close PWFILE;
my $valid_user=0;
my $tdepartment="";
for (@pwfile) {
    my $encrypsw=crypt($Password,"00");
    if (( m/^$Username:(.*):(.*):faculty:.*:$encrypsw$/ ) || ( m/^$Username:(.*):(.*):staff:.*:$encrypsw$/ )) {
        $valid_user=1;
        $tdepartment=$2;
    }
}

if ($valid_user != 1) {
    print "Location: ../html/rdproblemReporter.html\n\n";
    exit;
}

#Check department
my @departments=("CEN", "MEN", "SE", "EEN", "IS");
my $valid_dept=0;
my $vdepartment;
if ($department ne "") {
    $tdepartment=$department;
}
foreach $vdepartment (@departments) {
    if ($tdepartment eq $vdepartment) {
        $valid_dept=1;
    }
}
if ($valid_dept != 1) {
    # create the message body
    my $msgbody = qq|Hi!

$tdepartment group is currently not supported by ATA Tech Admins. Please report to right support staff.

Thank you for using ATA
    |;

    my $sendmail = "/usr/lib/sendmail -t";
    my $mailFrom = "From: nobody\@scu.edu\n";
    my $mailTo = "To: $email\n";
    my $subject = "Subject: Hi\n\n";

    print "Location: ../html/rdproblemReporter.html\n\n";

    # after composing, send message using sendmail
    #open(SENDMAIL, ">>../data/tmplog") or die "Cannot open $sendmail: $!";
    open(SENDMAIL, "| $sendmail") or die "Cannot open $sendmail: $!";
    printf(SENDMAIL "%s", $mailTo);
    printf(SENDMAIL "%s", $mailFrom);
    printf(SENDMAIL "%s", $subject);
    printf(SENDMAIL "%s\n", $msgbody);
    close(SENDMAIL);

    exit;
}

#Generate ProblemID
my $currentTime=time();
my $problemID=$machine."-".$currentTime;

#Log problem problemLOG
my $logfile="../data/problemLOG";
open(LOG, ">>$logfile") or die "Unable to open problemLOG file\n";
flock(LOG, 2);
printf(LOG "ProblemID:%s\n", $problemID);
printf(LOG "Name:%s\n", $name);
printf(LOG "Username:%s\n", $Username);
printf(LOG "Department:%s\n", $department);
printf(LOG "Email:%s\n", $email);
printf(LOG "Phone:%s\n", $phone);
printf(LOG "Machine:%s\n", $machine);
printf(LOG "Category:%s\n", $category);
printf(LOG "Issue:%s\n", $wellknownproblems);
printf(LOG "Description:%s\n", $description);
printf(LOG "Location:%s\n", $location);
printf(LOG "Time:%s\n", $time);
printf(LOG "\n");

close(LOG);

#Process for time info in problemDB
my @timearray=localtime();
my $year=$timearray[5]+1900;
my $hour=sprintf("%02d", $timearray[2]);
my $minutes=sprintf("%02d", $timearray[1]);
my $recordtime=$timearray[4]."/".$timearray[3]."/".$year.":".$hour.".".$minutes;

#If same machine has more than two problem reports on same day, assign same TA.
#Store for later use in problem assignment loadsahring
open(PROB_DB, "../data/problemDB") or die "cannot open it $!";
my @pdbfile=<PROB_DB>;
close(PROB_DB);

#Assign Tech Assistant using loadsharing
my @assistantDetail;
my @caseDetail;
my @schDetail;
my $CCAssistant="";
my $CCAssistantCase=0;
my @tdfile;
my @tschdfile;
my $techDBfile="../data/techDB";

my $eqlCaseID="";

for (@pdbfile) {
    my @caseDetail=split("\t");
    my @casedate=split(":", $caseDetail[2]);
    my $currentcasedate=$timearray[4]."/".$timearray[3]."/".$year;
    if (($caseDetail[1] eq $machine) && ($currentcasedate eq $casedate[0])) {
        $eqlCaseID = $caseDetail[0];
        last;
    }
}
#If same machine, assign same TA
if ( $eqlCaseID ne "" ) {
    open(TECHSCH_DB, "../data/problemSchedulesDB") or die "cannot open it $!";
    @tschdfile=<TECHSCH_DB>;
    close(TECHSCH_DB);
    for (@tschdfile) {
        @schDetail=split("\t");
        if (( $schDetail[0] eq "Schedule for ProblemID" ) || ( $schDetail[0] eq "" )) {
            next;
        }
        if ( $schDetail[0] eq $eqlCaseID ) {
             $CCAssistant = $schDetail[2];
             last;
        }
    }
    chomp($CCAssistant);
    open(TECHSCH_DB, ">>../data/problemSchedulesDB") or die "cannot open it $!";
    flock(TECHSCH_DB, 2);
    printf(TECHSCH_DB "%s\t%s\t%s\n", $problemID, $recordtime, $CCAssistant);
    close(TECHSCH_DB);

    # Update TechDB

    open(TECH_DB, "$techDBfile") or die "cannot open it $!";
    @tdfile = <TECH_DB>;
    close(TECH_DB);
    open(TECH_DB, ">$techDBfile") or die "cannot open it $!";
    flock(TECH_DB, 2);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if ( $assistantDetail[0] eq $CCAssistant ) {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], ++$assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "Tech Name" ) {
            printf(TECH_DB "%s\t%s\t%s", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "" ) {
            next;
        } else {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        }
    }
    close(TECH_DB);
#If hardware issue loadshare between hardware TAs
} elsif ( $category eq "Hardware" ) {
 # Assign Hardware TA with least cases & update his case count and schedule file
    open(TECH_DB, "$techDBfile") or die "cannot open it $!";
    @tdfile=<TECH_DB>;
    close(TECH_DB);
    open(TECH_DB, ">$techDBfile") or die "cannot open it $!";
    flock(TECH_DB, 2);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if (( $assistantDetail[0] eq "Tech Name" ) || ( $assistantDetail[0] eq "" )) {
            next;
        }
        if ( $assistantDetail[1] eq "Hardware" ) {
            if ( $CCAssistant eq "" ) {
                $CCAssistant = $assistantDetail[0];
                $CCAssistantCase = $assistantDetail[2];
            } elsif ( $assistantDetail[2] < $CCAssistantCase ) {
                $CCAssistant = $assistantDetail[0];
                $CCAssistantCase = $assistantDetail[2];
            }
        }
    }
    chomp($CCAssistant);
    open(TECHSCH_DB, ">>../data/problemSchedulesDB") or die "cannot open it $!";
    flock(TECHSCH_DB, 2);
    printf(TECHSCH_DB "%s\t%s\t%s\n", $problemID, $recordtime, $CCAssistant);
    close(TECHSCH_DB);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if ( $assistantDetail[0] eq $CCAssistant ) {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], ++$assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "Tech Name" ) {
            printf(TECH_DB "%s\t%s\t%s", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "" ) {
            next;
        } else {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        }
    }
    close(TECH_DB);
#If software issue loadshare between software TAs
} elsif ( $category eq "Software" ) {
 # Assign Software TA with least cases & update his case count and schedule file
    open(TECH_DB, "$techDBfile") or die "cannot open it $!";
    @tdfile=<TECH_DB>;
    close(TECH_DB);
    open(TECH_DB, ">$techDBfile") or die "cannot open it $!";
    flock(TECH_DB, 2);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if (( $assistantDetail[0] eq "Tech Name" ) || ( $assistantDetail[0] eq "" )) {
            next;
        }
        if ( $assistantDetail[1] eq "Software" ) {
            if ( $CCAssistant eq "" ) {
                $CCAssistant = $assistantDetail[0];
                $CCAssistantCase = $assistantDetail[2];
            } elsif ( $assistantDetail[2] < $CCAssistantCase ) {
                $CCAssistant = $assistantDetail[0];
                $CCAssistantCase = $assistantDetail[2];
            }
        }
    }
    chomp($CCAssistant);
    open(TECHSCH_DB, ">>../data/problemSchedulesDB") or die "cannot open it $!";
    flock(TECHSCH_DB, 2);
    printf(TECHSCH_DB "%s\t%s\t%s\n", $problemID, $recordtime, $CCAssistant);
    close(TECHSCH_DB);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if ( $assistantDetail[0] eq $CCAssistant ) {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], ++$assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "Tech Name" ) {
            printf(TECH_DB "%s\t%s\t%s", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "" ) {
            next;
        } else {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        }
    }
    close(TECH_DB);
#If category is not known for issue loadshare between all TAs
} else {
 # Assign TA with least cases and update his case count and schedule file
    open(TECH_DB, "$techDBfile") or die "cannot open it $!";
    @tdfile=<TECH_DB>;
    close(TECH_DB);
    open(TECH_DB, ">$techDBfile") or die "cannot open it $!";
    flock(TECH_DB, 2);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if (( $assistantDetail[0] eq "Tech Name" ) || ( $assistantDetail[0] eq "" )) {
            next;
        }
        if ( $CCAssistant eq "" ) {
            $CCAssistant = $assistantDetail[0];
            $CCAssistantCase = $assistantDetail[2];
        } elsif ( $assistantDetail[2] < $CCAssistantCase ) {
            $CCAssistant = $assistantDetail[0];
            $CCAssistantCase = $assistantDetail[2];
        }
    }
    chomp($CCAssistant);
    open(TECHSCH_DB, ">>../data/problemSchedulesDB") or die "cannot open it $!";
    flock(TECHSCH_DB, 2);
    printf(TECHSCH_DB "%s\t%s\t%s\n", $problemID, $recordtime, $CCAssistant);
    close(TECHSCH_DB);
    for (@tdfile) {
        @assistantDetail=split("\t");
        if ( $assistantDetail[0] eq $CCAssistant ) {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], ++$assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "Tech Name" ) {
            printf(TECH_DB "%s\t%s\t%s", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        } elsif ( $assistantDetail[0] eq "" ) {
            next;
        } else {
            printf(TECH_DB "%s\t%s\t%d\n", $assistantDetail[0], $assistantDetail[1], $assistantDetail[2]);
        }
    }
    close(TECH_DB);
}
#Update problemDB in single shot
open(PROB_DB, ">>../data/problemDB") or die "cannot open it $!";
flock(PROB_DB, 2);	
printf(PROB_DB "%s\t%s\t%s\t%s\ttechnician assigned\n", $problemID, $machine, $recordtime, $location);
close(PROB_DB);
#Send a mail to submitter

# create the message body

my $msgbody = qq|Hi!

$CCAssistant has been assigned to your problem $problemID reported as follows:

$wellknownproblems
$description

Thank you for using ATA
|;
# create message object and automatically compose new message

my $sendmail = "/usr/lib/sendmail -t";
my $mailFrom = "From: nobody\@scu.edu\n";
my $mailTo   = "To: $email\n";
my $subject   = "Subject: Hi\n\n";

#Redirect to success page
print "Location: ../html/successPR.html\n\n";

# after composing, send message using sendmail
#open(SENDMAIL, ">>../data/tmplog") or die "Cannot open $sendmail: $!";
open(SENDMAIL, "| $sendmail") or die "Cannot open $sendmail: $!";
printf(SENDMAIL "%s", $mailTo);
printf(SENDMAIL "%s", $mailFrom);
printf(SENDMAIL "%s", $subject);
printf(SENDMAIL "%s\n", $msgbody);
close(SENDMAIL);


