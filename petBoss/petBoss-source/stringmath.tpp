/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/

template<class RandomAccessIterator>
void StringMath::alphabeticalSort(RandomAccessIterator first,
                                  RandomAccessIterator last)
{
  sort(first, last, beforeInAlphabet);
}


