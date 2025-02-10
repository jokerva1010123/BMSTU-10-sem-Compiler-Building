package collections

type Stack[T any] struct {
	arr []T
}

func (s *Stack[T]) Push(value T) {
	s.arr = append(s.arr, value)
}

func (s *Stack[T]) Pop() T {
	l := len(s.arr)
	value := s.arr[l - 1]
	s.arr = s.arr[:l - 1]
	return value
}

func (s *Stack[T]) Top() T {
	l := len(s.arr)
	value := s.arr[l - 1]
	return value
}

func (s *Stack[T]) Empty() bool {
	return len(s.arr) == 0
}
