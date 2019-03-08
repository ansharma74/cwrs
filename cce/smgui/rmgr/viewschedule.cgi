#
#!/usr/local/bin/perl
###################################################################
#File:          viewschedule.cgi
#Description:   Perl cgi script to display lab schedule
#Course:        COEN276 Software Tools Design
#Project:       Project1
#Student:       Yu Wang
###################################################################
#

use strict;
use CGI ':standard';

# my_start_html generates "html" header instead.
sub my_start_html {
	print "<html>\n";
	print "<head>\n";
	print "<meta http-equiv=\"content-type\" content=\"text/html; charset=iso-8859-1\" />\n";
	print "<title>Schedule View</title>\n";
	print "</head>\n";
	print "<body bgcolor=\"#CCCCFF\" text=\"#000000\" alink=\"#0000CC\" vlink=\"#800000\">\n";
	print "\n";
}

sub my_end_html {
	print "\n";
	print "</body>\n";
	print "\n";
	print "</html>\n";
}

sub time_value {
	my $hour; my $minute; my $ampm;
	my $timeString;

	$timeString = shift;
	
	($hour,$minute,$ampm) = ($timeString =~ /(\d+):(\d{2})([apAP][mM])/);

	if ( ( $hour != 12 ) && ( lc($ampm) eq "pm" ) ) {
		$hour += 12;
	} 

	# debug
	#print "time_value: \"$timeString\" => $hour\n";

	return $hour;
}

sub generate_table {
	my $numOfLocations; my $numOfTimeSlots;
	my %reservedHash;
	my $location; my $group; my $numOfMachines; my $course; my $instructor; my $startTime; my $endTime; my $days;
	my %locationHash;
	my $machineID; my $os;
	my $reserveKey; my $key;

	open MACHINEDB, "../data/machinesDB" or die "$!";
	
	while ( <MACHINEDB> ) {
		# skip the comment or empty line
		next if /^#/ or /^$/;

		# get rid of the new line at the end
		chomp;

		# read one record
		($machineID,$os,$group,$location)=split(/:/,$_);

		# store locations and groups
		$locationHash{$location}{$group}=1;
	}

	$numOfLocations = scalar( keys( %locationHash ) );
	
	close MACHINEDB;

	# debug
	#print "numofLocations: " . $numOfLocations . "\n";
	#print "number of groups: " . scalar( %{ $locationHash{$location} } ) . "\n";
	
	open LABDB, "../data/lab_schedules" or die "$!";

	while ( <LABDB> ) {
		# skip the comment or empty line
		next if /^#/ or /^$/;

		# get rid of the new line at the end
		chomp;

		# read one record
		($location,$group,$numOfMachines,$course,$instructor,$startTime,$endTime,$days)=split(/\t/,$_);

		# debug
		#print "location: \"" . $location . "\", group: \"" . $group . "\", numOfMachines: \"" . $numOfMachines . "\", course: \"" . $course . "\", instructor: \"" . $instructor . "\", startTime: \"" . $startTime . "\", endTime: \"" . $endTime . "\", days: \"" . $days . "\"\n";

		# use another hash table to keep the record
		my $start; my $end;

		$start = time_value( $startTime );
		$end = time_value( $endTime );	
		
		# debug
		#print "start: $start, end: $end\n";

		while ( $start < $end ) {
			$reserveKey = $location.$group.$start.$days;
			$reservedHash{$reserveKey} = "$course: $instructor";
			$start++;
			
			# debug
			#print "reserveKey: \"$reserveKey\"\n";
		}
	}

	close LABDB;

	# debug
	#foreach $key ( keys %reservedHash ) {
	#	print "reservedHash{\"$key\"}: \"" . $reservedHash{$key} . "\"\n";
	#}

	# print the column header
	print "<table border=\"1\">\n";
	print "	<tr>\n";
	print "		<td>Time</td>\n";
	print "		<td colspan=\"2\">Monday</td>\n";
	print "		<td colspan=\"2\">Tuesday</td>\n";
	print "		<td colspan=\"2\">Wednesday</td>\n";
	print "		<td colspan=\"2\">Thursday</td>\n";
	print "		<td colspan=\"2\">Friday</td>\n";
	print "	</tr>\n";

	# create an array of locations
	my @locGroupHash; my @locName; my $idx;

	$idx = 0;
	foreach $key (sort( keys %locationHash )) {
		$locName[$idx] = $key;
		$locGroupHash[$idx++] = $locationHash{$key};
	}

	# print rows
	my @dayName=("Monday","Tuesday","Wednesday","Thursday","Friday");

	for ( my $time = 7; $time <= 22; $time++ ) {
		for ( my $loc = 0; $loc < $numOfLocations; $loc++ ) {
			print "	<tr>\n";

			# print the row header if this is the first location
			if ( $loc == 0 ) {
				my $ampm; my $hour;

				if ( $time >= 12 ) {
					$ampm = "PM";
					if ( $time != 12 ) {
						$hour = $time - 12;
					} else {
						$hour = $time;
					}
				} else {
					$ampm = "AM";
					$hour = $time;
				}

				$startTime = "$hour:00$ampm";

				print "		<td rowspan=\"4\">$startTime</td>\n";
			}

			# print the lab name and the group name(s) for each day
			for ( my $day = 0; $day < 5; $day++ ) {
				$location = $locName[$loc];
				my %groupHash = %{ $locGroupHash[$loc] };
				
				$idx = 0;
				print "		<td>$location</td>\n";
				print "		<td>";
				
				foreach $group (sort( keys %groupHash )) {
					if ( $idx++ != 0 ) {
						print "<br/>";
					}

					#$reserveKey = $location.$group.$startTime.$dayName[$day];
					$reserveKey = $location.$group.$time.$dayName[$day];
					
					my $info = $reservedHash{$reserveKey};

					# debug
					#print "info{\"$reserveKey\"}: \"$info\"\n";

					if ( $info eq "" ) {
						print "$group (available)";
					} else {
						print "<strong>$group ($info)</strong>";
					}
				}
				print "</td>\n";	
			}

			print "	</tr>\n";
		}
	}

	# end of table
	print "</table>\n";
}

###############################

# print CGI header
print header( {-type=>'text/html', -expires=>'now'} );

# print html header
my_start_html();

# print some description about the table
print "<table border=0 width=\"100%\" height=\"47\">\n";
print "<tr>\n";
print "<td bgcolor=\"#CC0099\" height=\"45\" width=\"100%\" align=\"left\" valign=\"middle\">\n";
print "<b><font size=\"5\">The reserved groups are in <strong>bold</strong>.</font></b>\n";
print "</td>\n";
print "</tr>\n";
print "</table>\n";

# generate html table from the lab database
generate_table();

# print html footer
my_end_html();
