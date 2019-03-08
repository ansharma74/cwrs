#
#!/usr/local/bin/perl
###################################################################
# File:		 scheduleLab.cgi
# Description:	 Perl cgi script to process lab reserving 
# Course:        COEN276        Software Tools Design
# Project:       Project 1
# Student:	 Yu  Wang
####################################################################

use strict;
use CGI ':standard';

#declare variables

my $Username=param("Username");
my $Password=param("Password");
my $input_fullname=param("Fullname");
my $input_dept=param("Department");
my $input_phone=param("Phone");
my $input_email=param("Email");
my $input_course=param("Course");
my $input_day=param("Days");
my $input_number=param("NumofStudent");
my $input_os=param("os-select");
my $input_starttime=param("StartTime");
my $input_endtime=param("ToTime");
my $input_group;

my $schedule_day;
my $schedule_group;
my $schedule_location;
my $schedule_starttime;
my $schedule_endtime;
my $schedule_course;
my $schedule_instructor;
my $schedule_number;
my @s_os;
my @s_group;
my %s_schedule;

my $db_location;
my $db_group;
my $db_os;
my $db_id;
my $db_number;
my @db_number;
my $db_group_key;
my $db_num_value;
my $db_location_value;
my %db_num;
my %db_location;
my @db_temp_group;
my @db_temp_location;

my $remain_number;
my $key;
my $value;
my $used_number;
my $temp_dbnumber;
my $total_machine;
my @reserved_group;
my %available_location;
my %available_num;
my @available_location;
my @temp_group;
my $allocatedLoc;
my @allocatedGroups;
my $machineCount;
my $loc;
my $num;

#Debug Block
#print header({-type=>'text/plain', -expire=>'now'});
#print "Info received $Username $Password $input_dept\n";

#Check username/password
open PWFILE, "../data/userFile" or die "cannot open userFile $!" ;
my @pwfile=<PWFILE>;
close PWFILE;
my $valid_user=0;
my $tdepartment="";
for (@pwfile) {
    my $encrypsw=crypt($Password,"00");
    #print "Before Usercheck $Username $Password $encrypsw\n";
    if (( m/^$Username:(.*):(.*):faculty:.*:$encrypsw$/ ) || ( m/^$Username:(.*):(.*):staff:.*:$encrypsw$/ )) {
        #print "Usercheck $Username $Password $encrypsw\n";
        $valid_user=1;
        $tdepartment=$2;
    }
}

if ($valid_user != 1) {
    #print "Usercheck failed $Username $Password $input_dept\n";
    print "Location: ../html/rdlabreserve.html\n\n";
    exit;
}

#Check department
my @departments=("CEN", "MEN", "SE", "EEN", "IS");
my $valid_dept=0;
my $vdepartment;
if ($input_dept ne "") {
    $tdepartment=$input_dept;
}
foreach $vdepartment (@departments) {
    if ($tdepartment eq $vdepartment) {
        $valid_dept=1;
    }
}
if ($valid_dept != 1) {
    # create the message body
    my $msgbody = qq|Hi!

$tdepartment group is currently not supported by ATA Tech Admins. Please report
to right support staff.

Thank you for using ATA
    |;

    my $sendmail = "/usr/lib/sendmail -t";
    my $mailFrom = "From: nobody\@scu.edu\n";
    my $mailTo = "To: $input_email\n";
    my $subject = "Subject: Hi\n\n";

    print "Location: ../html/rdlabreserve.html\n\n";

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
#update lab_schedule
#write the record into lab_schedules database text file
sub fnWrite() {
	open (LABSCHEDULES, ">>../data/lab_schedules") || die "cannot open $!";
	# exclusive lock
	flock (LABSCHEDULES,2); 
	# write to the end of the file
	seek (LABSCHEDULES, 0, 2);
	for (my $j=0; $j<scalar(@allocatedGroups); $j++) {
	print LABSCHEDULES  $allocatedLoc."\t".$allocatedGroups[$j]."\t".$input_number."\t".$input_course."\t".$Username."\t".$input_starttime."\t".$input_endtime."\t".$input_day."\n";
	}
	close (LABSCHEDULES);
}

#
#print header( {-type=>'text/html', -expire=>'now'} );

open LABSCHEDULES,"../data/lab_schedules" or die "cannot open database file $!";
# read only
flock (LABSCHEDULES,1);

#partition time slice

while (<LABSCHEDULES>) {
	chomp;
	#skip empty line
	next if /^$/;
	#skip comment line
	next if /^#/;

	($schedule_location, $schedule_group,$schedule_number,$schedule_course, $schedule_instructor, $schedule_starttime, $schedule_endtime, $schedule_day)=split(/\t/,$_);
	#debug line
	#print "schedule_day=".$schedule_day."schedule_starttime".$schedule_starttime.$input_starttime. $input_day."\n";
	
	####################################################################
	#find all groups from the lab_schedules textfile within the input
	#duration
	#set array: @reserved_group
	#####################################################################
	if (((($schedule_starttime >= $input_starttime) && ($schedule_starttime < $input_endtime)) || (($schedule_endtime > $input_starttime) && ($schedule_endtime <= $input_endtime))) && ($schedule_day eq $input_day)) {
		#print "schedule_group ".$schedule_group."\n";
		push (@reserved_group, $schedule_group);
	}
} 
close (LABSCHEDULES);

#debug line:
#print "reserved group: \n";
#print @reserved_group;

open MACHINESDB, "../data/machinesDB" or die "cannont open $!";
flock (MACHINESDB,1);
while (<MACHINESDB>) {
	chomp;
	next if /^$/;
	next if /^#/;
	($db_id,$db_os,$db_group,$db_location)=split(/:/,$_);

	###################################################################
	#set associative arrays: %db_location	%db_num
	#as input_os = db_os;
	#use group as the key;  location and number of the machine as the 
	#value respectively to set %db_location and %db_num hash
	#
	#calculate total number of the machine with same OS in machinesDB
	####################################################################

	if (($db_os eq $input_os) || ($input_os eq "does not matter")) {
		#set hash %db_location (key: group; value: location)
		$db_group_key=$db_group;
		$db_location_value=$db_location;
		$db_location{$db_group_key}=$db_location_value;

		#set hash %db_num (key: group; value: the number of machine in same group)
		$db_num{$db_group_key}+=1;
		$db_num_value=$db_num{$db_group_key};
		$db_num{$db_group_key}=$db_num_value;

	}
}
close (MACHINESDB);

#debug line:
#print "db_num hash \n";
#print %db_num;
#print "db_location:\n";
#print %db_location;

#############################################################################
#From the reserved_group array, we can know the reserved groups with the 
#input duration; 
#From the hashes (%db_num or %db-location), we know the groups with the same
#OS as required machines
#Based on the above information, we can get all the possible groups that has 
#same OS and available to be reserved as the required duration.
##############################################################################

%available_location=%db_location;
%available_num=%db_num;

foreach $db_group_key (keys %db_location) {
	#print "group key: ".$db_group_key."\n";
	#print scalar(@reserved_group);
	for (my $i=0; $i<scalar(@reserved_group); $i++) {
	#print "number of group:".scalar(@reserved_group)."\n";
		if ($reserved_group[$i] eq $db_group_key) {
			#print "reserved_group:"."\"".$reserved_group[$i]."\" db_group_key: "."\"".$db_group_key."\"\n";
			delete $available_location{$db_group_key};
			delete $available_num{$db_group_key};
			#print %available_location;
			#print %available_num;
			if (%available_location==()) {
	                    print "Location: ../html/failedSL.html\n\n";
			    exit;
			}
		}
	}
}
##############################################################################
#we have got all groups that have the same OS as required and are available
#to be reserved. In the next block, we will use a loop to find the exact
#group and lab that fit the user's requirement. Taking required machine number
#and group location into consideration.
##############################################################################

$machineCount=$input_number;

foreach $db_group_key (keys %available_num) {
	$num=$available_num{$db_group_key};
	$loc=$available_location{$db_group_key};

	if (($allocatedLoc ne "") && ($allocatedLoc ne $loc)) {
		next;
	}

	$machineCount-=$num;
	push (@allocatedGroups, $db_group_key);

	$allocatedLoc=$available_location{$db_group_key};

	if ($machineCount <= 0) {
	        print "Location: ../cgi-bin/lab_confirm.cgi?Group=@allocatedGroups&Lab=$loc\n\n";
		#print "the ";
		#print @allocatedGroups;
		#print " in the ".$loc." is reserved.";
		fnWrite();
		exit;
	}

}

if ($machineCount > 0) {
	print "Location: ../html/failedSL.html\n\n";
} 




