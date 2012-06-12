#!/usr/bin/perl 

use warnings;
use strict;

use Term::ReadLine;
use FileHandle;
use Data::Dumper;
use Fcntl qq(SEEK_SET);



my $TTY_DEVICE = "/dev/ttyUSB2";
#my $TTY_DEVICE = "testfifo";

my $fr = fork();
die "could not fork!" if (!defined $fr);

if ($fr != 0) {

    my $hadcon_out = new FileHandle("<$TTY_DEVICE");
    if(!defined $hadcon_out) {
	print "could not open $TTY_DEVICE\n";
	exit;
    }
    $hadcon_out=~s/\r/\n/;
    while(<$hadcon_out>) {
	print STDERR "HADCON: $_";
    }
    print STDERR "no more input on $TTY_DEVICE\n";
    system(reset);
    exit;
}


#exit;
# parent


my $term = Term::ReadLine->new('HADCON Commands');
my $prompt = "Enter command to HADCON: ";

my $OUT = $term->OUT || \*STDOUT;

my $fh = new FileHandle(".hadcon_history", "+>>");


seek($fh,0,SEEK_SET);
my @hist = <$fh>;

#print Dumper \@hist;

foreach (@hist) {
    $term->addhistory($_) if /\S/;
}


while ( defined (my $input = $term->readline($prompt)) ) {
    $_=$input;
    chomp; 
#    $_=uc($_);
    my $c= qq|echo $_ >$TTY_DEVICE |; 
    my $r=qx($c);

    #my $res = eval($_);
    #warn $@ if $@;
    #print $OUT $res, "\n" unless $@;
    if ($input=~/\S/) {
	$term->addhistory($input);
	print $fh "$input\n";
    }

    select (undef, undef, undef, 0.15);
    my $size = -s $TTY_DEVICE;
    #print "size: $size\n";
    #exit;
    #while ( -s $TTY_DEVICE > 0) { print "waiting\n";}
    #sleep 1;

}

system(reset);
