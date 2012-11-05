#!/usr/bin/perl

# Removes all line breaks and comment strings (comments defined as starting with '<') in a given file
# output is to stdout

if (scalar(@ARGV) == 1) {
	open(INPUT, "<$ARGV[0]") or die ("Couldn't open file $ARGV[0]");
} else {
	print "ERROR: This script takes one argument, the name of the file to flatten";
	exit 0;
}

while(<INPUT>) {
	my($line) = $_;
	chomp($line);
	
	if($line =~ m/^>/) { ; }
	else {
		print($line);
	}
}

close(INPUT);
