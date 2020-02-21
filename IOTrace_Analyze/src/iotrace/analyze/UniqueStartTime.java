package iotrace.analyze;

public class UniqueStartTime implements Comparable<UniqueStartTime> {
	private static long number = 0;

	private long tailBreak;
	private long startTime;

	public UniqueStartTime(long startTime) {
		this.startTime = startTime;
		tailBreak = number;
		number++;
	}

	@Override
	public int compareTo(UniqueStartTime o) {
		int cmp = Long.valueOf(startTime).compareTo(o.startTime);

		if (cmp == 0) {
			if (tailBreak < o.tailBreak) {
				return -1;
			} else if (tailBreak > o.tailBreak) {
				return 1;
			} else {
				return 0;
			}
		}

		return cmp;
	}
}
