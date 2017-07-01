#!/usr/local/bin/perl
use strict; use warnings;
use Digest::SHA ();
use File::stat;
##########################
# BLOCK - chop file evenly
my ($dir) = @ARGV;
my $block = 10000;

open(my $lfh, '<', LIST) or die "no list\n";
open(my $bLfh, '>>', 'BATCH') or dir "no batch log\n";

my @list = readline $lfh; chomp @list;
foreach my $i (@list)
{
  my $path = $dir.$i;
  open(my $ifh, '<', $path) or print $bLfh "open ERK $i\n";
  binmode $ifh;

  my $st = stat($path);
  my $total = $st->size;

  my @file = <$ifh>;
  my $count = @file;
  my $pos = 0;

  while ($pos < $count)
  {

    $count += $pos+$block;
  }
}
