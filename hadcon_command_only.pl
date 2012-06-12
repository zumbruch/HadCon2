#!/usr/bin/perl 

use warnings;
use strict;

use Term::ReadLine;
use FileHandle;
use Data::Dumper;
use Fcntl qq(SEEK_SET);



my $TTY_DEVICE = "/dev/hadcon2";
#my $TTY_DEVICE = "testfifo";

my $fh_out;

$SIG{USR1} = \&catch_usr1;

my $arg = $ARGV[0];

#print "arg $arg\n";

if($arg) {
    $TTY_DEVICE = $arg;
}

print "opening device: $TTY_DEVICE\n";


my $term = Term::ReadLine->new('HADCON Commands');
my $prompt = "command: ";

my $OUT = $term->OUT || \*STDOUT;

my $fh = new FileHandle($ENV{'HOME'} . "/.hadcon_history", "+>>");


seek($fh,0,SEEK_SET);
my @hist = <$fh>;

#print Dumper \@hist;

foreach (@hist) {
    $term->addhistory($_) if /\S/;
}


select (undef, undef, undef, 0.10);

#my $or = open $fh_out, ">$TTY_DEVICE";
#if(!$or || !$fh_out) {
#    print "could not open $TTY_DEVICE for writing: $!\n";
#}

while ( defined (my $input = $term->readline($prompt)) ) {
    $_=$input;
    chomp; 
# sub commands are case sensitive ! 
#    $_=uc($_);
#    my $c= qq|echo $_ >$TTY_DEVICE |; 
#    my $r=qx($c);
    
    #print "command sent $_\n";
    if(!$fh_out) {
	$fh_out = open_device($TTY_DEVICE);
    }

    my $pr = print $fh_out $_ . "\n";
    if(!$pr) {
	print "could not write to $TTY_DEVICE: $!\n";
	close($fh_out);
	$fh_out = open_device($TTY_DEVICE);
    }

    #open $fh_out, ">$TTY_DEVICE";


    #my $res = eval($_);
    #warn $@ if $@;
    #print $OUT $res, "\n" unless $@;
    if($input=~/quit/ || $input=~/exit/ ) {
	print "exiting...\n";
	my $c = 'pkill -f "/usr/bin/perl -w .*slurp_serial"';
	system($c);
	exit;
    }

    if ($input=~/\S/) {
	$term->addhistory($input);
	print $fh "$input\n";
    }

    #select (undef, undef, undef, 0.05);
    #my $size = -s $TTY_DEVICE;
    #print "size: $size\n";
    #exit;
    #while ( -s $TTY_DEVICE > 0) { print "waiting\n";}
    #sleep 1;

}

sub open_device {
    my ($dev) = @_;

    my $fh;

    my $or;
    while (!$or) {
	print "trying to open device\n";
	$or = open $fh, ">" , "$dev";
	if(!$or) {
	    print "$!\n";
	    sleep 1;
	}
    }

    return $fh;
}

sub catch_usr1 {

    print "got usr1 signal, closing output file\n";
    close($fh_out);
    undef $fh_out;
}
