use strict;

# Base frequency
my $base_freq = 1000000;

# sqrt12(2)
my $note_diff = 1.0594630943592952645618252949463;

my @notes = ('C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B');

# Start note is E4
my $freq = 440/$note_diff/$note_diff/$note_diff/$note_diff/$note_diff;
my $note = 4;
my $octave = 4;

my @result = ();

foreach my $i (1..32)
{
    print "$notes[$note]$octave = $freq Hz, ";
    
    my $divisor = int($base_freq/$freq);
    my $divisor2 = $divisor + 1;
    my $frq1 = $base_freq/$divisor;
    my $frq2 = $base_freq/$divisor2;

    print "div($divisor): $frq1, div($divisor2): $frq2 -> ";
    if (abs($freq - $frq2) < abs($freq - $frq1))
    {
        $divisor = $divisor2;
    }

    print "$divisor\n";

    push(@result, $divisor);

    # Next note
    $freq = $freq*$note_diff;
    if (++$note == 12)
    {
        $note = 0;
        ++$octave;
    }
}

print "Results:\n";
print "static const uint16_t pm_noteDivisors[] PROGMEM =\n";
print "{\n";
print "    ";

foreach my $i (@result)
{
    print "$i, ";
}

print "\n};\n";
