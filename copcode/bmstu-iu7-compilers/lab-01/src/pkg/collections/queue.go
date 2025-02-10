package collections

type Queue[T any] struct {
	arr []T
}

func (s *Queue[T]) Push(value T) {
	s.arr = append(s.arr, value)
}

func (s *Queue[T]) Pop() T {
	value := s.arr[0]
	s.arr = s.arr[1:]
	return value
}

func (s *Queue[T]) Empty() bool {
	return len(s.arr) == 0
}
