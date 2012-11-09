#!/usr/bin/perl

# Read in a file line-by-line, swap characters every once in a while
# output is to stdout

my(@INS_CHARS) = ("A","C","G","T");
my($NTH_LINE_TO_SCRAMBLE) = 6;

if (scalar(@ARGV) == 1) {
	open(INPUT, "<$ARGV[0]") or die ("Couldn't open file $ARGV[0]");
} else {
	print "ERROR: This script takes one argument, the name of the file to flatten";
	exit 0;
}

my($linecount) = 0;
my($inschar_idx) = 0;
my($chars_scrambled) = 0;

while(<INPUT>) {
	my($line) = $_;
	chomp($line);
	
	if($line =~ m/^>/) { ; }
	else {
		if($linecount % $NTH_LINE_TO_SCRAMBLE == 0) {
			$line = substr($line, 0, 29) . $INS_CHARS[$inschar_idx] . substr($line, 30, length($line));
			$inschar_idx = ($inschar_idx+1) % (scalar @INS_CHARS);
			$chars_scrambled++;
		}
		
		print($line);
		$linecount++;
	}
}

print STDERR "\nScrambed $chars_scrambled characters.\n";

close(INPUT);
