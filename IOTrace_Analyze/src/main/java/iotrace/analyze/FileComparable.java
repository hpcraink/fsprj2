package iotrace.analyze;

public interface FileComparable<T> extends Comparable<T> {

	@Override
	public abstract int hashCode();

	@Override
	public abstract boolean equals(Object obj);

	@Override
	public abstract int compareTo(T arg0);

	@Override
	public abstract String toString();
	
	public abstract String toFileName();
}
