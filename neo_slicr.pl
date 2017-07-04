sub slicr
{
	my ($i) = @_;
	my $st = stat($i);
	my $total = $st->size;
	open(my $ifh, '<', "$i") || die "Cant open $i: $!\n";
	binmode($ifh);
	my $sha = file_digest($i);
	my $block = 0; my $position = 0;
	while ($position < $total)
	{
		my $size = int(rand(99999));
		if ($position + $size >= $total)
			{ $size = $total - $position; }
		read($ifh, $block, $size);
		my $fh = new_block($i, $sha, $path, $block);
		print $fh $block;
		close $fh;
		$count += $size;
	}
	print $Lfh "YAY $i\n";
}
