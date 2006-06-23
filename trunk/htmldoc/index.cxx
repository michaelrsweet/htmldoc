//
// "$Id$"
//
//   Indexing methods for HTMLDOC, a HTML document processing program.
//
//   Copyright 1997-2006 by Easy Software Products.
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
//       Hollywood, Maryland 20636 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   index_build()                   - Build an index from a file...
//   index_doc()                     - Build an index from a list of phrases...
//   hdIndex::hdIndex()              - Create an index...
//   hdIndex::~hdIndex()             - Destroy an index...
//   hdIndex::scan()                 - Scan a document tree for index matches...
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
// Local functions...
//

static hdTree	*index_doc(hdTree *doc, int num_phrases, const char **phrases);


//
// 'index_build()' - Build an index from a file...
//

hdTree *				// O - Index tree
index_build(hdTree     *doc,		// I - Document
            const char *words)		// I - File containing phrases
{
  FILE		*fp;			// File pointer
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

  if ((fp = fopen(words, "rb")) == NULL)
    return (NULL);

  while (file_gets(phrase, sizeof(phrase), fp))
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
  ind = index_doc(doc, num_phrases, phrases);

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
// 'index_doc()' - Build an index from a list of phrases...
//

hdTree *				// O - Index tree
index_doc(hdTree     *doc,		// I - Document
          int        num_phrases,	// I - Number of phrases
	  const char **phrases)		// I - Phrases
{
  int		i, j, k, n;		// Looping vars
  hdIndex	*inddata;		// Index data
  hdIndexPhrase	**p;			// Index phrase
  int		ch;			// Current character
  char		*topic;			// Current topic
  hdTree	*ind,			// Index
		*title,			// Title of index
		*link,			// Link target for index
		*indptr,		// Pointer into index
		*indnode,		// Node in index
		*indlink;		// Link node in index
  char		s[1024];		// String value


  // Create the index data...
  inddata = new hdIndex(num_phrases, phrases);
  inddata->scan(doc);
  inddata->sort_topics();

#if 0
  for (i = 0, p = inddata->phrases; i < inddata->num_phrases; i ++, p ++)
  {
     printf("%s:", p[0]->topic);
     for (j = 0; j < p[0]->num_words; j ++)
       printf(" %s", p[0]->words[j]);

     printf(" [%d matches]\n", p[0]->num_matches);

     for (j = 0; j < p[0]->num_matches; j ++)
       printf("    #%d%p (%s)\n", j + 1, p[0]->matches[j],
              htmlGetAttr(p[0]->matches[j], "name"));
  }
#endif // 0

  // Create a root node for the index...
  ind = htmlAddTree(NULL, HD_ELEMENT_BODY, NULL);
  htmlSetAttr(ind, "class", (hdChar *)"HD_INDEX");
  htmlUpdateStyle(ind, ".");

  title = htmlAddTree(ind, HD_ELEMENT_H1, NULL);
  htmlSetAttr(title, "class", (hdChar *)"HD_INDEX");
  htmlUpdateStyle(title, ".");

  link = htmlAddTree(title, HD_ELEMENT_A, NULL);
  htmlSetAttr(link, "name", (hdChar *)"HD_INDEX");
  htmlUpdateStyle(link, ".");

  htmlAddTree(link, HD_ELEMENT_NONE, (hdChar *)IndexTitle);

  indptr = ind;

  // Loop through the index data and create the index...
  for (i = 0, p = inddata->phrases, ch = -1, topic = NULL;
       i < inddata->num_phrases;
       i ++, p ++)
  {
    // Skip phrases with no matches...
    if (p[0]->num_matches == 0)
      continue;

    // See if this is a new initial letter...
    if (ch != toupper(p[0]->topic[0]))
    {
      // Get the new initial letter...
      ch = toupper(p[0]->topic[0]);

      // Create an "h2" heading with the letter...
      indnode = htmlAddTree(ind, HD_ELEMENT_H2, NULL);
      htmlSetAttr(indnode, "class", (hdChar *)"HD_INDEX");
      htmlUpdateStyle(indnode, ".");

      indlink = htmlAddTree(indnode, HD_ELEMENT_A, NULL);
      snprintf(s, sizeof(s), "HD_INDEX_%c", ch);
      htmlSetAttr(indlink, "name", (hdChar *)s);
      htmlUpdateStyle(indlink, ".");

      s[0] = ch;
      s[1] = '\0';
      htmlAddTree(indlink, HD_ELEMENT_NONE, (hdChar *)s);
    }

    if (topic != p[0]->topic)
    {
      if (p[0]->topic == p[0]->words[0])
      {
        // No topic...
        topic  = NULL;
	indptr = ind;
      }
      else
      {
        // Phrase with topic...
        topic = p[0]->topic;

	// Create an paragraph with the topic...
	indnode = htmlAddTree(ind, HD_ELEMENT_P, NULL);
	htmlSetAttr(indnode, "class", (hdChar *)"HD_INDEX");
	htmlUpdateStyle(indnode, ".");

        htmlAddTree(indnode, HD_ELEMENT_NONE, (hdChar *)topic);

	// Then the unordered list for the items...
	indptr = htmlAddTree(ind, HD_ELEMENT_UL, NULL);
	htmlSetAttr(indptr, "class", (hdChar *)"HD_INDEX");
	htmlUpdateStyle(indptr, ".");
      }
    }

    indnode = htmlAddTree(indptr, indptr == ind ? HD_ELEMENT_P : HD_ELEMENT_LI,
                          NULL);
    htmlSetAttr(indnode, "class", (hdChar *)"HD_INDEX");
    htmlUpdateStyle(indnode, ".");

    htmlAddTree(indnode, HD_ELEMENT_NONE, (hdChar *)(p[0]->words[0]));
    for (j = 1; j < p[0]->num_words; j ++)
    {
      snprintf(s, sizeof(s), " %s", p[0]->words[j]);
      htmlAddTree(indnode, HD_ELEMENT_NONE, (hdChar *)s);
    }

    for (j = 0, n = 0; j < p[0]->num_matches; j = k)
    {
      // Add the link...
      htmlAddTree(indnode, HD_ELEMENT_NONE, (hdChar *)" ");

      // Add a link for the page...
      indlink = htmlAddTree(indnode, HD_ELEMENT_A, NULL);
      snprintf(s, sizeof(s), "#%s", htmlGetAttr(p[0]->matches[j], "NAME"));
      htmlSetAttr(indlink, "href", (hdChar *)s);
      htmlSetAttr(indlink, "_HD_FULL_HREF", (hdChar *)s);
      indlink->link = indlink;
      htmlUpdateStyle(indlink, ".");

      n ++;
      sprintf(s, "%d", n);
      htmlAddTree(indlink, HD_ELEMENT_NONE, (hdChar *)s);

      for (k = j + 1; k < p[0]->num_matches; k ++)
        if (p[0]->matches[j] != p[0]->matches[k])
	  break;

      if (k < p[0]->num_matches)
	htmlAddTree(indnode, HD_ELEMENT_NONE, (hdChar *)",");
    }
  }

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
  hdChar	*ptr;			// Pointer to data
  int		number;			// Target number
  char		name[255];		// Target name


  // with the minimal optimization of a hashing table.  
  // Sort the phrases by word...
  sort_words();

  // Scan the document...
  for (target = NULL, number = 0; t; t = htmlRealNext(t))
  {
    // This method basically uses a "brute force" searching algorithm.
    // The only optimizations we use use are a hash table that tells us
    // where to start looking for matches, and we exit early if we
    // find a phrase that appears later in the comparison.
    //
    // Index entries point to the most resent link target (<a name="foo">)
    // and we make an attempt to update this link when a match is found.
    // (i.e. if a paragraph contains a target, we use that target...)

    if (t->element == HD_ELEMENT_A && htmlGetAttr(t, "NAME"))
      target = t;
    else if (t->element == HD_ELEMENT_NONE)
    {
      // Check this node for matches...
      for (ptr = t->data; isspace(*ptr); ptr ++);

      i = tolower(*ptr);
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
	    // We found a match; if there is no target in the current
	    // paragraph, add one...
	    if (!target)
	    {
	      // Create the node...
	      target = htmlNewTree(NULL, HD_ELEMENT_A, NULL);
	      sprintf(name, "HD_INDEX_%06d", number ++);
	      htmlSetAttr(target, "name", (hdChar *)name);

              // Reparent the current node under the target node...
              target->prev       = t->prev;
	      target->next       = t->next;
	      target->parent     = t->parent;
	      target->link       = t->link;
	      target->style      = t->style;
	      target->child      = t;
	      target->last_child = t;

              if (target->prev)
	        target->prev->next = target;
	      else
	        target->parent->child = target;

              if (target->next)
	        target->next->prev = target;
	      else
	        target->parent->last_child = target;

	      htmlUpdateStyle(target, ".");

	      t->prev   = NULL;
	      t->next   = NULL;
	      t->parent = target;
	      t->style  = target->style;
            }

            // Add the link to the list of matches...
	    phrases[i]->add_match(target);
	  }

          i ++;
	}
      }
    }
    else if (!t->style || t->style->display != HD_DISPLAY_INLINE)
      target = NULL;
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
  for (i = 0; i < num_words && t != NULL; t = htmlRealNext(t))
  {
    // Only compare text fragments; stop when we hit a block boundary.
    if (t->element != HD_ELEMENT_NONE)
    {
      if (t->style->display != HD_DISPLAY_INLINE)
        break;
      else
        continue;
    }

    // Do a case-insensitive comparison, ignoring trailing punctuation
    // chars and skipping leading whitespace...
    for (a = (char *)t->data; isspace(*a & 255); a ++);
    for (b = words[i]; *a && *b; a ++, b ++)
      if (tolower(*a & 255) != tolower(*b & 255))
        break;

    if (*b || (!ispunct(*a & 255) && !isspace(*a & 255) && *a))
    {
      if (tolower(*a & 255) < tolower(*b & 255))
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
hdIndexPhrase::compare_words(
    const hdIndexPhrase **a,		// I - First phrase
    const hdIndexPhrase **b)   		// I - Second phrase
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
// End of "$Id$".
//
