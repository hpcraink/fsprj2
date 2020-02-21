package iotrace.analyze;

import java.util.Collection;
import java.util.SortedSet;
import java.util.TreeSet;

public class KeyValueTreeNode implements Comparable<KeyValueTreeNode> {
	private double value;
	private String key;

	private KeyValueTreeNode parent = null;
	private SortedSet<KeyValueTreeNode> children = new TreeSet<>();
	private KeyValueTreeNode otherChild = null;

	public KeyValueTreeNode(double value, String key) {
		super();
		this.value = value;
		this.key = key;
	}

	public double getValue() {
		return value;
	}

	public String getKey() {
		return key;
	}

	public KeyValueTreeNode getParent() {
		return parent;
	}

	public SortedSet<KeyValueTreeNode> getChildren() {
		SortedSet<KeyValueTreeNode> tmp = new TreeSet<>();
		tmp.addAll(children);
		if (otherChild != null) {
			tmp.add(otherChild);
		}
		return tmp;
	}

	public void addChild(KeyValueTreeNode child) {
		child.parent = this;
		children.add(child);
	}

	public void addChildren(Collection<KeyValueTreeNode> children) {
		for (KeyValueTreeNode child : children) {
			child.parent = this;
			this.children.add(child);
		}
	}

	public void addOtherChild(double value) {
		if (value != 0) {
			if (otherChild == null) {
				otherChild = new KeyValueTreeNode(value, "other");
			} else {
				otherChild.value += value;
			}
		}
	}

	public boolean hasParent() {
		if (parent == null) {
			return false;
		} else {
			return true;
		}
	}

	public boolean hasChildren() {
		if (!children.isEmpty() || otherChild != null) {
			return true;
		} else {
			return false;
		}
	}

	public boolean hasRealChildren() {
		return !children.isEmpty();
	}

	@Override
	public int compareTo(KeyValueTreeNode o) {
		Double d1 = value;
		Double d2 = o.value;

		int cmp = d1.compareTo(d2);
		if (cmp == 0) {
			return key.compareTo(o.key);
		} else {
			return cmp;
		}
	}
}
