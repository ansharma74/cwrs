#For code viewing
#!/usr/local/bin/perl
####################################################################
#description:	 Perl cgi script to generate xml file 
#
# Student:	 Yu  Wang
####################################################################

use strict;
use CGI ':standard';

my $s_location;
my $s_group;
my $s_number;
my $s_course;
my $s_instructor;
my $s_starttime;
my $s_endtime;
my $s_day;

open LABSCHEDULES,"lab_schedules.txt" or die "cannot open database file $!";
# read only
flock (LABSCHEDULES,1);
open SCHEDULES_XML,">schedules.xml" or die "cannot open database file $!";
flock (SCHEDULES_XML,2);

#partition time slice
print SCHEDULES_XML "<labschedules>\n";

while (<LABSCHEDULES>) {
	chomp;
	#skip empty line
	next if /^$/;
	#skip comment line
	next if /^#/;

	($s_location, $s_group, $s_number,$s_course, $s_instructor, $s_starttime, $s_endtime, $s_day)=split(/\t/,$_);

	print SCHEDULES_XML "\t<schedule>\n";
	print SCHEDULES_XML "\t\t<lab>";
	print SCHEDULES_XML $s_location;
	print SCHEDULES_XML "</lab>\n";
	print SCHEDULES_XML "\t\t<group>";
	print SCHEDULES_XML $s_group;
	print SCHEDULES_XML "</group>\n";
	print SCHEDULES_XML "\t\t<number>";
	print SCHEDULES_XML $s_number;
	print SCHEDULES_XML "</number>\n";
	print SCHEDULES_XML "\t\t<course>";
	print SCHEDULES_XML $s_course;
	print SCHEDULES_XML "</course>\n";
	print SCHEDULES_XML "\t\t<instructor>";
	print SCHEDULES_XML $s_instructor;
	print SCHEDULES_XML "</instructor>\n";
	print SCHEDULES_XML "\t\t<starttime>";
	print SCHEDULES_XML $s_starttime;
	print SCHEDULES_XML "</starttime>\n";
	print SCHEDULES_XML "\t\t<endtime>";
	print SCHEDULES_XML $s_endtime;
	print SCHEDULES_XML "</endtime>\n";
	print SCHEDULES_XML "\t\t<day>";
	print SCHEDULES_XML $s_day;
	print SCHEDULES_XML "</day>\n";
	print SCHEDULES_XML "\t</schedule>\n";
	
}
print SCHEDULES_XML "</labschedules>\n";


