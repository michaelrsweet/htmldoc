//
// "$Id: index.cxx,v 1.3 2002/04/03 18:12:56 mike Exp $"
//
//   Indexing methods for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2002 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdTree::build_index()           - Build an index from a file...
//   hdTree::build_index()           - Build an index from a list of words...
//   hdIndex::hdIndex()              - Create an index...
//   hdIndex::~hdIndex()             - Destroy an index...
//   hdIndex::scan()                 - Scan a document tree for index
//                                     matches...
//   hdIndex::sort_topics()          - Sort phrases by topic.
//   hdIndex::sort_words()           - Sort phrases by words.
//   hdIndexPhrase::hdIndexPhrase()  - Create a phrase for indexing.
//   hdIndexPhrase::~hdIndexPhrase() - Delete a phrase.
//   hdIndexPhrase::add_match()      - Add a match to a phrase...
//   hdIndexPhrase::compare_topics() - Compare two phrases, ordering by the
//                                     topic.
//   hdIndexPhrase::compare_tree()   - Compare a phrase to tree nodes...
//   hdIndexPhrase::compare_words()  - Compare two phrases, ordering by the
//                                     word.
//

//
// Include necessary headers...
//

#include "htmldoc.h"
#include "hdstring.h"


//
// Structure for word searching structure...
//

struct hdIndexPhrase
{
  char		*topic;			// Topic in index
  int		num_words;		// Number of words in phrase
  char		**words;		// Words
  int		num_matches,		// Number of matches
		alloc_matches;		// Allocated matches
  hdTree	**matches;		// Matches

  hdIndexPhrase(const char *phrase);
  ~hdIndexPhrase();

  void		add_match(hdTree *t);
  static int	compare_topics(const hdIndexPhrase **a, const hdIndexPhrase **b);
  int		compare_tree(hdTree *t);
  static int	compare_words(const hdIndexPhrase **a, const hdIndexPhrase **b);
};

struct hdIndex
{
  int		num_phrases;		// Number of phrases
  hdIndexPhrase	**phrases;		// Phrase table
  int		hash[256];		// Hash table into array

  hdIndex(int np, const char **p);
  ~hdIndex();

  void		scan(hdTree *t);
  void		sort_topics();
  void		sort_words();
};


//
// 'hdTree::build_index()' - Build an index from a file...
//

hdTree *				// O - Index tree
hdTree::build_index(hdStyleSheet *css,	// I - Style sheet
                    hdFile       *fp)	// I - File containing phrases
{
  int		i,			// Looping var
		num_phrases,		// Number of phrases
		alloc_phrases;		// Allocated phrases
  const char	**phrases,		// Phrases array
		**temp;			// Temp array pointer
  char		phrase[1024];		// Phrase (list) from file
  hdTree	*ind;			// Index tree


  // Read the phrase list from a file...
  phrases       = NULL;
  num_phrases   = 0;
  alloc_phrases = 0;

  while (fp->gets(phrase, sizeof(phrase)) != NULL)
  {
    if (num_phrases >= alloc_phrases)
    {
      temp = new const char *[alloc_phrases + 100];

      if (alloc_phrases)
      {
        memcpy(temp, phrases, sizeof(const char *) * alloc_phrases);
	delete[] phrases;
      }

      phrases       = temp;
      alloc_phrases += 100;
    }

    phrases[num_phrases] = strdup(phrase);
    num_phrases ++;
  }

  // Build the index...
  ind = build_index(css, num_phrases, phrases);

  // Free phrases as needed...
  if (num_phrases > 0)
  {
    for (i = 0; i < num_phrases; i ++)
      free((void *)phrases[i]);

    delete[] phrases;
  }

  // Return the index...
  return (ind);
}


//
// 'hdTree::build_index()' - Build an index from a list of phrases...
//

hdTree *				// O - Index tree
hdTree::build_index(hdStyleSheet *css,	// I - Style sheet
                    int          num_phrases,
					// I - Number of phrases
		    const char   **phrases)
					// I - Phrases
{
  int		i, j;			// Looping vars
  hdTree	*t,			// Current node
		*tnext,			// Next node
		*tlink,			// Pointer to link
		*ttemp;			// Temporary node pointer
  hdIndex	*inddata;		// Index data
  hdIndexPhrase	**p;			// Index phrase
  hdTree	*ind,			// Table of contents
		*indptr,		// Pointer into index
		*indnode,		// Node in index
		*indlink;		// Link node in index
  const char	*val;			// Attribute value


  // Create the index data...
  inddata = new hdIndex(num_phrases, phrases);

  inddata->scan(this);

  for (i = 0, p = inddata->phrases; i < inddata->num_phrases; i ++, p ++)
  {
     printf("%s:", p[0]->topic);
     for (j = 0; j < p[0]->num_words; j ++)
       printf(" %s", p[0]->words[j]);

     printf(" [%d matches]\n", p[0]->num_matches);

     for (j = 0; j < p[0]->num_matches; j ++)
       printf("    #%d%p (%s)\n", j + 1, p[0]->matches[j],
              p[0]->matches[j]->get_attr("name"));
  }

  // Create a root node for the index...
  ind = new hdTree(HD_ELEMENT_BODY);
  ind->set_attr("class", "INDEX");
  ind->style = css->find_style(ind);

#if 0
  // Scan the document tree for headings...
  for (indptr = ind, t = this; t; t = tnext)
  {
    // Point to the next node in the tree...
    tnext = t->real_next();

    // See if we have a heading...
    if (t->element >= HD_ELEMENT_H1 && t->element <= HD_ELEMENT_H6)
    {
      // Yes, handle it...
      tnext    = t->next;
      newlevel = t->element - HD_ELEMENT_H1;

      // If the new level is not the same as the old level, (un)indent
      // as needed...
      while (newlevel > level)
      {
        // Indent lower...
        level ++;

        numbers[level] = 0;

	indnode = new hdTree(HD_ELEMENT_UL, NULL, indptr);
	indnode->set_attr("class", "INDEX");
	indnode->style = css->find_style(indnode);
	indptr = indnode;
      }
      
      while (newlevel < level)
      {
        // Unindent higher...
        level --;

	indptr = indptr->parent;
      }

      // Bump the chapter number as needed...
      if (level == 0)
        chapter ++;

      // See if we have a new format or number value...
      if ((val = t->get_attr("VALUE")) != NULL)
        numbers[level] = atoi(val);
      else
        numbers[level] ++;

      if ((val = t->get_attr("TYPE")) != NULL)
        formats[level] = *val;

      // Do we need heading numbers?
      if (numbers)
      {
        // Yes, format the number...
        s[0]             = '\0';
	s[sizeof(s) - 1] = '\0';

        for (i = 0, sptr = s; i <= level; i ++, sptr += strlen(s))
	{
	  hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                         formats[i], numbers[i]);

          if (i < level)
	    strncat(sptr, ".", sizeof(s) - (sptr - s) - 1);
        }

        indnumber = new hdTree(HD_ELEMENT_NONE, s);
	indnumber->insert(t);
	indnumber->style = t->style;
	indnumber->compute_size(css);

	if (indnumber->next)
	  indnumber->next->whitespace = 1;
      }

      // Do we need to add a link target?
      if ((tlink = t->find(HD_ELEMENT_A)) == NULL)
      {
        // No link node; insert one...
	tlink = new hdTree(HD_ELEMENT_A);
	tlink->child      = t->child;
	tlink->last_child = t->last_child;

	for (ttemp = t->child; ttemp != NULL; ttemp = ttemp->next)
	  ttemp->parent = tlink;

        t->child      = tlink;
	t->last_child = tlink;
      }

      if (tlink->get_attr("name") == NULL)
      {
        // Add a named link...
	strcpy(s, "HD_");
	s[sizeof(s) - 1] = '\0';

        for (i = 0, sptr = s + 3; i <= level; i ++, sptr += strlen(s))
	{
	  hdGlobal.format_number(sptr, sizeof(s) - (sptr - s),
	                         formats[i], numbers[i]);

          if (i < level)
	    strncat(sptr, "_", sizeof(s) - (sptr - s) - 1);
        }

	tlink->set_attr("name", s);
      }

      // Then add a new node for the heading...
      indnode = new hdTree(level == 0 ? HD_ELEMENT_P : HD_ELEMENT_LI, NULL, indptr);
      indnode->set_attr("class", "INDEX");
      indnode->style = css->find_style(indnode);

      // And a node for the link...
      snprintf(s, sizeof(s), "#%s", tlink->get_attr("name"));

      indlink = new hdTree(HD_ELEMENT_A, "", indnode);
      indlink->set_attr("href", s);
      indlink->set_attr("class", "INDEX");
      indlink->style = css->find_style(indlink);

      // Copy the text to the index...
      t->copy_text(indlink);

      // Finally, add a pseudo-attribute for the chapter number...
      snprintf(s, sizeof(s), "%d", chapter);
      tlink->set_attr("_HD_CHAPTER", s);
      indlink->set_attr("_HD_CHAPTER", s);
    }
  }
#endif // 0

  return (ind);
}


//
// 'hdIndex::hdIndex()' - Create an index...
//

hdIndex::hdIndex(int        np,		// I - Number of phrases
                 const char **p)	// I - Phrases
{
  int	i;				// Looping var


  // Initialize things...
  num_phrases = np;

  if (np <= 0)
    return;

  // Allocate memory for the phrases...
  phrases = new hdIndexPhrase *[np];

  for (i = 0; i < np; i ++)
    phrases[i] = new hdIndexPhrase(p[i]);
}


//
// 'hdIndex::~hdIndex()' - Destroy an index...
//

hdIndex::~hdIndex()
{
  int	i;				// Looping var


  // Free memory as needed...
  if (num_phrases > 0)
  {
    for (i = 0; i < num_phrases; i ++)
      delete phrases[i];

    delete[] phrases;
  }
}


//
// 'hdIndex::scan()' - Scan a document tree for index matches...
//

void
hdIndex::scan(hdTree *t)		// I - Document to scan
{
  int		i,			// Looping var
		result;			// Result of comparison
  hdTree	*target,		// Current link target...
		*temp;			// New link target...


  // with the minimal optimization of a hashing table.  
  // Sort the phrases by word...
  sort_words();

  // Scan the document...
  for (target = NULL; t != NULL; t = t->real_next())
  {
    // This method basically uses a "brute force" searching algorithm.
    // The only optimizations we use use are a hash table that tells us
    // where to start looking for matches, and we exit early if we
    // find a phrase that appears later in the comparison.
    //
    // Index entries point to the most resent link target (<a name="foo">)
    // and we make an attempt to update this link when a match is found.
    // (i.e. if a paragraph contains a target, we use that target...)

    if (t->element == HD_ELEMENT_A && t->get_attr("name") != NULL)
      target = t;
    else if (t->element == HD_ELEMENT_NONE)
    {
      // Check this node for matches...
      i = tolower(t->data[0] & 255);
      i = hash[i];

      if (i >= 0)
      {
        // OK, this starting character has a match; check it...
	result = -1;
	temp   = NULL;

	while (i < num_phrases)
	{
	  if ((result = phrases[i]->compare_tree(t)) < 0)
	    break;
          else if (result == 0)
	  {
	    // We found a match; see if the current paragraph contains
	    // a link target...
	    if (temp == NULL && t->parent != NULL &&
		(temp = t->parent->find(HD_ELEMENT_A)) != NULL)
	      if (temp->get_attr("name") != NULL)
		target = temp;

            // Add the link to the list of matches...
	    phrases[i]->add_match(target);
	  }

          i ++;
	}
      }
    }
  }
}


//
// 'hdIndex::sort_topics()' - Sort phrases by topic.
//

void
hdIndex::sort_topics()
{
  int	i,				// Looping var
	ch;				// Start character


  // Sort the phrases...
  if (num_phrases > 1)
    qsort(phrases, num_phrases, sizeof(hdIndexPhrase *),
          (hdCompareFunc)hdIndexPhrase::compare_topics);

  // Initialize the hash table...
  memset(hash, -1, sizeof(hash));

  for (i = 0; i < num_phrases; i ++)
  {
    ch = tolower(phrases[i]->topic[0] & 255);

    if (hash[ch] < 0)
      hash[ch] = i;
  }
}


//
// 'hdIndex::sort_words()' - Sort phrases by words.
//

void
hdIndex::sort_words()
{
  int	i,				// Looping var
	ch;				// Start character


  // Sort the phrases...
  if (num_phrases > 1)
    qsort(phrases, num_phrases, sizeof(hdIndexPhrase *),
          (hdCompareFunc)hdIndexPhrase::compare_words);

  // Initialize the hash table...
  memset(hash, -1, sizeof(hash));

  for (i = 0; i < num_phrases; i ++)
  {
    if (phrases[i]->num_words == 0)
      continue;

    ch = tolower(phrases[i]->words[0][0] & 255);

    if (hash[ch] < 0)
      hash[ch] = i;
  }
}


//
// 'hdIndexPhrase::hdIndexPhrase()' - Create a phrase for indexing.
//

hdIndexPhrase::hdIndexPhrase(const char *phrase)
					// I - Phrase text
{
  int	alloc_words;			// Number of words allocated
  char	*s,				// Pointer to copy of phrase
	**temp;				// New words pointer


  // Start by clearing the structure...
  memset(this, 0, sizeof(hdIndexPhrase));

  // Then parse the phrase; we can either have:
  //
  //     [topic string] word word word ...
  //
  // or:
  //
  //     word word word ...
  //
  // The topic string is not included in the index search, but
  // is only used to group the search words under the indicated
  // topic.  If no topic is supplied, then the first word is used.
  //
  // I'm sure at some point we'll add support for XML-based
  // word lists, but for now we just do this (simpler) mechanism...

  while (isspace(*phrase))
    phrase ++;

  if (phrase[0] == '[')
  {
    // OK, phrase starts with a topic string.
    s     = strdup(phrase + 1);
    topic = s;

    // Scan forward and strip the trailing ]...
    while (*s && *s != ']')
      s ++;

    if (*s == ']')
      *s++ = '\0';
  }
  else
  {
    // Phrase starts with a word...
    s     = strdup(phrase);
    topic = s;
  }

  // Now add words to the end of the string...
  for (alloc_words = 0; *s;)
  {
    // Skip leading whitespace...
    while (isspace(*s))
      s ++;

    if (!*s)
      break;

    // Allocate memory for the words as needed...
    if (num_words >= alloc_words)
    {
      temp = new char *[alloc_words + 4];

      if (alloc_words)
      {
        memcpy(temp, words, sizeof(char *) * alloc_words);
	delete[] words;
      }

      words       = temp;
      alloc_words += 4;
    }

    words[num_words] = s;
    num_words ++;

    // Then skip past the word...
    while (*s && !isspace(*s))
      s ++;

    // Nul-terminate as needed...
    if (*s)
      *s++ = '\0';
  }
}


//
// 'hdIndexPhrase::~hdIndexPhrase()' - Delete a phrase.
//

hdIndexPhrase::~hdIndexPhrase()
{
  if (topic)
    free((void *)topic);

  if (matches)
    delete[] matches;
}


//
// 'hdIndexPhrase::add_match()' - Add a match to a phrase...
//

void
hdIndexPhrase::add_match(hdTree *t)	// I - Link node to add
{
  int		i;			// Looping var
  hdTree	**temp;			// Temporary match array pointer


  // Don't add NULL matches...
  if (t == NULL)
    return;

  // See if the match has already been added...
  for (i = 0; i < num_matches; i ++)
    if (matches[i] == t)
      return;

  // Allocate memory as needed...
  if (num_matches >= alloc_matches)
  {
    temp = new hdTree *[alloc_matches + 10];

    if (alloc_matches)
    {
      memcpy(temp, matches, sizeof(hdTree *) * alloc_matches);
      delete[] matches;
    }

    matches       = temp;
    alloc_matches += 10;
  }

  matches[num_matches] = t;
  num_matches ++;
}


//
// 'hdIndexPhrase::compare_topics()' - Compare two phrases, ordering by the topic.
//

int					// O - Result of comparison
hdIndexPhrase::compare_topics(const hdIndexPhrase **a,
					// I - First phrase
                              const hdIndexPhrase **b)
			      		// I - Second phrase
{
  int	result;				// Result of comparison


  if ((result = strcasecmp(a[0]->topic, b[0]->topic)) != 0)
    return (result);
  else
    return (compare_words(a, b));
}


//
// 'hdIndexPhrase::compare_tree()' - Compare a phrase to tree nodes...
//

int					// O - Result of comparison
hdIndexPhrase::compare_tree(hdTree *t)	// I - Current node
{
  int		i;			// Looping var
  const char	*a,			// First string
		*b;			// Second string


  // Scan the words...
  for (i = 0; i < num_words && t != NULL; t = t->real_next())
  {
    // Only compare text fragments; stop when we hit a block boundary.
    if (hdElIsBlock(t->element))
      break;
    else if (t->element != HD_ELEMENT_NONE)
      continue;

    // Do a case-insensitive comparison, ignoring trailing punctuation
    // chars...
    for (a = t->data, b = words[i]; *a && *b; a ++, b ++)
      if (tolower(*a) != tolower(*b))
        break;

    if (*b || (!ispunct(*a) && *a))
    {
      if (tolower(*a) < tolower(*b))
        return (-1);
      else
        return (1);
    }

    i ++;
  }

  // OK, didn't get a result yet; if we checked all the words,
  // then return 0 for a match.  Otherwise, return 1...
  if (i < num_words)
    return (-1);
  else
    return (0);
}


//
// 'hdIndexPhrase::compare_words()' - Compare two phrases, ordering by the word.
//

int					// O - Result of comparison
hdIndexPhrase::compare_words(const hdIndexPhrase **a,
					// I - First phrase
                             const hdIndexPhrase **b)
			      		// I - Second phrase
{
  int	i;				// Looping var...
  int	result;				// Result of comparison


  // Compare each word and return as soon as there is a difference...
  for (i = 0; i < a[0]->num_words && i < b[0]->num_words; i ++)
    if ((result = strcasecmp(a[0]->words[i], b[0]->words[i])) != 0)
      return (result);

  // If all comparisons matched, return the difference in the
  // number of words...
  return (a[0]->num_words - b[0]->num_words);
}


//
// End of "$Id: index.cxx,v 1.3 2002/04/03 18:12:56 mike Exp $".
//
