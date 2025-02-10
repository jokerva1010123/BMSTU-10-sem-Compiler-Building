package collections

type Set[T comparable] []T

func (s Set[T]) Dupl() Set[T] {
	return append([]T(nil), s...)
}
func (s *Set[T]) Add(value T) {
	for _, v := range *s {
		if v == value {
			return
		}
	}

	*s = append(*s, value)
}

func (s Set[T]) ToArray() []T {
	return s
}

func (s Set[T]) Unite(o Set[T]) Set[T] {
	var newSet Set[T]

	for _, v := range s {
		newSet.Add(v)
	}
	for _, v := range o.ToArray() {
		newSet.Add(v)
	}

	return newSet
}

func (s Set[T]) Subtract(o Set[T]) Set[T] {
	var newSet Set[T]

	for _, v := range s {
		if !o.Contains(v) {
			newSet.Add(v)
		}
	}

	return newSet
}

func (s Set[T]) Size() int {
	return len(s)
}

func (s Set[T]) Equals(o Set[T]) bool {
	s1 := s.Unite(o)
	s2 := o.Unite(s)

	if len(s1.ToArray()) == len(s.ToArray()) && len(o.ToArray()) == len(s2.ToArray()) {
		return true
	}
	return false
}

func (s Set[T]) Contains(v T) bool {
	for _, i := range s {
		if i == v {
			return true
		}
	}

	return false
}
