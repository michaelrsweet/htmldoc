//
// "$Id: render-table.cxx,v 1.7 2003/12/06 04:01:35 mike Exp $"
//
//   Table rendering methods for HTMLDOC, a HTML document processing
//   program.
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
//

//
// Include necessary headers.
//

//#define DEBUG*/
#include "debug.h"
#include "htmldoc.h"
#include "hdstring.h"
#include <stdlib.h>


//
// 'hdRender::render_table()' - Parse a table.
//

void
hdRender::render_table(hdTree   *t,		// I  - Table
                       hdMargin &m,		// I  - Current margins
		       float    &x,		// IO - Current X position
		       float    &y,		// IO - Current Y position
		       int      &page)		// IO - Current page
{
  int		col,
		row,
		tcol,
		colspan,
		rowspan,
		num_cols,
		num_rows,
		alloc_rows,
		regular_cols,
		tempspace,
		col_spans[HD_MAX_COLUMNS],
		row_spans[HD_MAX_COLUMNS];
  char		col_fixed[HD_MAX_COLUMNS];
  float		col_lefts[HD_MAX_COLUMNS],
		col_rights[HD_MAX_COLUMNS],
		col_width,
		col_widths[HD_MAX_COLUMNS],
		col_swidths[HD_MAX_COLUMNS],
		col_min,
		col_mins[HD_MAX_COLUMNS],
		col_smins[HD_MAX_COLUMNS],
		col_pref,
		col_prefs[HD_MAX_COLUMNS],
		col_height,
		cellpadding,
		cellspacing,
		border,
		border_left,
		border_size,
		bottom,
		top,
		width,
		pref_width,
		span_width,
		regular_width,
		actual_width,
		table_width,
		min_width,
		temp_width,
		row_y, temp_y,
		temp_bottom,
		temp_top;
  int		row_page, temp_page;
  const char	*var,
		*height_var;			// Row HEIGHT variable
  hdTree	*temprow,
		*tempcol,
		*tempnext,
		***cells;
  int		do_valign;			// True if we should do vertical alignment of cells
  float		row_height,			// Total height of the row
		temp_height;			// Temporary holder
  int		cell_page[HD_MAX_COLUMNS],	// Start page for cell
		cell_endpage[HD_MAX_COLUMNS];	// End page for cell
  float		cell_y[HD_MAX_COLUMNS],		// Row or each cell
		cell_endy[HD_MAX_COLUMNS],	// Row or each cell
		cell_height[HD_MAX_COLUMNS],	// Height of each cell in a row
		span_heights[HD_MAX_COLUMNS];	// Height of spans
  hdRenderNode	*cell_bg[HD_MAX_COLUMNS];	// Background rectangles
  hdRenderNode	*cell_start[HD_MAX_COLUMNS];	// Start of the content for a cell in the row
  hdRenderNode	*cell_end[HD_MAX_COLUMNS];	// End of the content for a cell in a row
  const char	*bgcolor;
  float		rgb[3],
		bgrgb[3];
  hdMargin	*cell_margin;


  DEBUG_puts("\n\nTABLE");

  DEBUG_printf(("render_table(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                t, left, right, x, y, page));

  if (t->child == NULL)
    return;   // Empty table...

  rgb[0] = t->style->color[0];
  rgb[1] = t->style->color[1];
  rgb[2] = t->style->color[2];

  // Figure out the # of rows, columns, and the desired widths...
  memset(col_spans, 0, sizeof(col_spans));
  memset(col_fixed, 0, sizeof(col_fixed));
  memset(col_widths, 0, sizeof(col_widths));
  memset(col_swidths, 0, sizeof(col_swidths));
  memset(col_mins, 0, sizeof(col_mins));
  memset(col_smins, 0, sizeof(col_smins));
  memset(col_prefs, 0, sizeof(col_prefs));

  cells = NULL;

  if ((var = t->get_attr("WIDTH")) != NULL)
  {
    if (var[strlen(var) - 1] == '%')
      table_width = atof(var) * (m.width()) / 100.0f;
    else
      table_width = atoi(var) * 72.0f / css->ppi;
  }
  else
    table_width = m.width();

  bottom = m.bottom();
  top    = m.top();

  DEBUG_printf(("table_width = %.1f\n", table_width));

  if ((var = t->get_attr("CELLPADDING")) != NULL)
    cellpadding = atoi(var);
  else
    cellpadding = 1.0f;

  if ((var = t->get_attr("CELLSPACING")) != NULL)
    cellspacing = atoi(var);
  else
    cellspacing = 0.0f;

  if ((var = t->get_attr("BORDER")) != NULL)
  {
    if ((border = atof(var)) == 0.0 && var[0] != '0')
      border = 1.0f;

    cellpadding += border;
  }
  else
    border = 0.0f;

  if (border == 0.0f && cellpadding > 0.0f)
  {
    // Ah, the strange table formatting nightmare that is HTML.
    // Netscape and MSIE assign an invisible border width of 1
    // pixel if no border is specified...
    cellpadding += 1.0f;
  }

  border_size = border - 1.0f;

  cellspacing *= 72.0f / css->ppi;
  cellpadding *= 72.0f / css->ppi;
  border      *= 72.0f / css->ppi;
  border_size *= 72.0f / css->ppi;

  temp_bottom = bottom - cellpadding;
  temp_top    = top + cellpadding;

  memset(row_spans, 0, sizeof(row_spans));
  memset(span_heights, 0, sizeof(span_heights));

  for (temprow = t->child, num_cols = 0, num_rows = 0, alloc_rows = 0;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;

    if (temprow->element == HD_ELEMENT_CAPTION)
      render_doc(temprow, m, x, y, page);
    else if (temprow->element == HD_ELEMENT_TR ||
             ((temprow->element == HD_ELEMENT_TBODY || temprow->element == HD_ELEMENT_THEAD ||
               temprow->element == HD_ELEMENT_TFOOT) && temprow->child != NULL))
    {
      // Descend into table body as needed...
      if (temprow->element == HD_ELEMENT_TBODY || temprow->element == HD_ELEMENT_THEAD ||
          temprow->element == HD_ELEMENT_TFOOT)
        temprow = temprow->child;

      // Figure out the next row...
      if ((tempnext = temprow->next) == NULL)
        if (temprow->parent->element == HD_ELEMENT_TBODY ||
            temprow->parent->element == HD_ELEMENT_THEAD ||
            temprow->parent->element == HD_ELEMENT_TFOOT)
          tempnext = temprow->parent->next;

      // Allocate memory for the table as needed...
      if (num_rows >= alloc_rows)
      {
        alloc_rows += HD_ALLOC_ROWS;

        if (alloc_rows == HD_ALLOC_ROWS)
	  cells = (hdTree ***)malloc(sizeof(hdTree **) * alloc_rows);
	else
	  cells = (hdTree ***)realloc(cells, sizeof(hdTree **) * alloc_rows);

        if (cells == (hdTree ***)0)
	{
	  hdGlobal.progress_error(HD_ERROR_OUT_OF_MEMORY,
                         "Unable to allocate memory for table!");
	  return;
	}
      }	

      if ((cells[num_rows] = (hdTree **)calloc(sizeof(hdTree *), HD_MAX_COLUMNS)) == NULL)
      {
	hdGlobal.progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for table!");
	return;
      }

#ifdef DEBUG
      printf("BEFORE row %d: num_cols = %d\n", num_rows, num_cols);

      if (num_rows)
        for (col = 0; col < num_cols; col ++)
	  printf("    col %d: row_spans[] = %d\n", col, row_spans[col]);
#endif // DEBUG

      // Figure out the starting column...
      if (num_rows)
      {
	for (col = 0, rowspan = 9999; col < num_cols; col ++)
	  if (row_spans[col] < rowspan)
	    rowspan = row_spans[col];

	for (col = 0; col < num_cols; col ++)
	  row_spans[col] -= rowspan;

	for (col = 0; row_spans[col] && col < num_cols; col ++)
          cells[num_rows][col] = cells[num_rows - 1][col];
      }
      else
        col = 0;

      for (tempcol = temprow->child;
           tempcol != NULL && col < HD_MAX_COLUMNS;
           tempcol = tempcol->next)
        if (tempcol->element == HD_ELEMENT_TD || tempcol->element == HD_ELEMENT_TH)
        {
	  // Handle colspan and rowspan stuff...
          if ((var = tempcol->get_attr("COLSPAN")) != NULL)
            colspan = atoi(var);
          else
            colspan = 1;

          if ((var = tempcol->get_attr("ROWSPAN")) != NULL)
	  {
            row_spans[col] = atoi(var);

	    for (tcol = 1; tcol < colspan; tcol ++)
              row_spans[col + tcol] = row_spans[col];
          }

          // Compute the cell size...
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min,
	                            &col_pref, &col_height);
          if ((var = tempcol->get_attr("WIDTH")) != NULL)
	  {
	    if (var[strlen(var) - 1] == '%')
              col_width -= 2.0 * cellpadding - cellspacing;
	  }
	  else if (tempcol->get_attr("NOWRAP") != NULL)
	    col_width = col_pref;
	  else
	    col_width = 0.0f;

          tempcol->height = col_height;

	  DEBUG_printf(("%d,%d: colsp=%d, rowsp=%d, width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
	                col, num_rows, colspan, row_spans[col], col_width,
			col_min, col_pref, col_height));

          // Add widths to columns...
          if (colspan > 1)
          {
	    if (colspan > col_spans[col])
	      col_spans[col] = colspan;

	    if (col_width > col_swidths[col])
	      col_swidths[col] = col_width;

	    if (col_min > col_smins[col])
	      col_smins[col] = col_min;
          }
	  else
	  {
	    if (col_width > 0.0f)
	      col_fixed[col] = 1;

	    if (col_width > col_widths[col])
	      col_widths[col] = col_width;

	    if (col_pref > col_prefs[col])
	      col_prefs[col] = col_pref;

	    if (col_min > col_mins[col])
	      col_mins[col] = col_min;
          }

	  while (colspan > 0 && col < HD_MAX_COLUMNS)
	  {
            cells[num_rows][col] = tempcol;
            col ++;
            colspan --;
          }

          while (row_spans[col] && col < num_cols)
	  {
            cells[num_rows][col] = cells[num_rows - 1][col];
	    col ++;
	  }
        }

      if (col > num_cols)
        num_cols = col;

#ifdef DEBUG
      printf("AFTER row %d: num_cols = %d\n", num_rows, num_cols);

      for (col = 0; col < num_cols; col ++)
        printf("    col %d: row_spans[] = %d\n", col, row_spans[col]);
#endif // DEBUG

      num_rows ++;

      for (col = 0; col < num_cols; col ++)
        if (row_spans[col])
	  row_spans[col] --;
    }
  }

  // Now figure out the width of the table...
  if ((var = t->get_attr("WIDTH")) != NULL)
  {
    if (var[strlen(var) - 1] == '%')
      width = atof(var) * (m.right() - m.left()) / 100.0f;
    else
      width = atoi(var) * 72.0f / css->ppi;
  }
  else
  {
    for (col = 0, width = 0.0; col < num_cols; col ++)
      width += col_prefs[col];

    width += (2 * cellpadding + cellspacing) * num_cols - cellspacing;

    if (width > m.width())
      width = m.width();
  }

  // Compute the width of each column based on the printable width.
  DEBUG_printf(("\nTABLE: %dx%d\n\n", num_cols, num_rows));

  actual_width  = (2 * cellpadding + cellspacing) * num_cols -
                  cellspacing;
  regular_width = (width - actual_width) / num_cols;

  DEBUG_printf(("    width = %.1f, actual_width = %.1f, regular_width = %.1f\n\n",
                width, actual_width, regular_width));
  DEBUG_puts("    Col  Width   Min     Pref    Fixed?");
  DEBUG_puts("    ---  ------  ------  ------  ------");

#ifdef DEBUG
  for (col = 0; col < num_cols; col ++)
    printf("    %-3d  %-6.1f  %-6.1f  %-6.1f  %s\n", col, col_widths[col],
           col_mins[col], col_prefs[col], col_fixed[col] ? "YES" : "NO");

  puts("");
#endif // DEBUG

  // The first pass just handles columns with a specified width...
  DEBUG_puts("PASS 1: fixed width handling\n");

  for (col = 0, regular_cols = 0; col < num_cols; col ++)
    if (col_widths[col] > 0.0f)
    {
      if (col_mins[col] > col_widths[col])
        col_widths[col] = col_mins[col];

      actual_width += col_widths[col];
    }
    else
      regular_cols ++;

  DEBUG_printf(("    actual_width = %.1f, regular_cols = %d\n\n", actual_width,
                regular_cols));

  // Pass two uses the "preferred" width whenever possible, and the
  // minimum otherwise...
  DEBUG_puts("PASS 2: preferred width handling\n");

  for (col = 0, pref_width = 0.0f; col < num_cols; col ++)
    if (col_widths[col] == 0.0f)
      pref_width += col_prefs[col];

  DEBUG_printf(("    pref_width = %.1f\n", pref_width));

  if (pref_width > 0.0f)
  {
    if ((regular_width = (width - actual_width) / pref_width) < 0.0f)
      regular_width = 0.0f;
    else if (regular_width > 1.0f)
      regular_width = 1.0f;

    DEBUG_printf(("    regular_width = %.1f\n", regular_width));

    for (col = 0; col < num_cols; col ++)
      if (col_widths[col] == 0.0f)
      {
	pref_width = col_prefs[col] * regular_width;
	if (pref_width < col_mins[col])
          pref_width = col_mins[col];

	if ((actual_width + pref_width) > width)
	{
          if (col == (num_cols - 1) && (width - actual_width) >= col_mins[col])
	    col_widths[col] = width - actual_width;
	  else
	    col_widths[col] = col_mins[col];
	}
	else
          col_widths[col] = pref_width;

        DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));

	actual_width += col_widths[col];
      }
  }

  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));

  // Pass three enforces any hard or minimum widths for COLSPAN'd
  // columns...
  DEBUG_puts("PASS 3: colspan handling\n\n");

  for (col = 0; col < num_cols; col ++)
  {
    DEBUG_printf(("    col %d, colspan %d\n", col, col_spans[col]));

    if (col_spans[col] > 1)
    {
      for (colspan = 0, span_width = 0.0f; colspan < col_spans[col]; colspan ++)
        span_width += col_widths[col + colspan];

      pref_width = 0.0f;

      if (span_width < col_swidths[col])
        pref_width = col_swidths[col];
      if (span_width < col_smins[col] && pref_width < col_smins[col])
        pref_width = col_smins[col];

      for (colspan = 0; colspan < col_spans[col]; colspan ++)
        if (col_fixed[col + colspan])
	{
          span_width -= col_widths[col + colspan];
	  pref_width -= col_widths[col + colspan];
	}

      DEBUG_printf(("    col_swidths=%.1f, col_smins=%.1f, span_width=%.1f, pref_width=%.1f\n",
                    col_swidths[col], col_smins[col], span_width, pref_width));

      if (pref_width > 0.0f && pref_width > span_width)
      {
        if (span_width >= 1.0f)
	{
          // Expand cells proportionately...
	  regular_width = pref_width / span_width;

	  for (colspan = 0; colspan < col_spans[col]; colspan ++)
	    if (!col_fixed[col + colspan])
	    {
	      actual_width -= col_widths[col + colspan];
	      col_widths[col + colspan] *= regular_width;
	      actual_width += col_widths[col + colspan];

              DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
	    }
        }
	else
	{
	  // Divide the space up equally between columns, since the
	  // colspan area is always by itself... (this hack brought
	  // to you by Yahoo! and their single cell tables with
	  // colspan=2 :)

	  regular_width = pref_width / col_spans[col];

	  for (colspan = 0; colspan < col_spans[col]; colspan ++)
	  {
	    actual_width += regular_width;
	    col_widths[col + colspan] += regular_width;

            DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
	  }
	}
      }
    }
  }

  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));

  // Pass four divides up the remaining space amongst the columns...
  DEBUG_puts("PASS 4: divide remaining space, if any...\n");

  if (width > actual_width)
  {
    regular_width = (width - actual_width) / num_cols;

    for (col = 0; col < num_cols; col ++)
    {
      col_widths[col] += regular_width;
      DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
    }
  }
  else
    width = actual_width;

  DEBUG_puts("");

  // The final pass is only run if the width > table_width...
  DEBUG_puts("PASS 5: Squeeze table as needed...");

  if (width > table_width)
  {
    // Squeeze the table to fit the requested width or the printable width
    // as determined at the beginning...
    for (col = 0, min_width = -cellspacing; col < num_cols; col ++)
      min_width += col_mins[col];

    DEBUG_printf(("    table_width = %.1f, width = %.1f, min_width = %.1f\n",
                  table_width, width, min_width));

    temp_width = table_width - min_width;
    if (temp_width < 0.0f)
      temp_width = 0.0f;

    width -= min_width;
    if (width < 1.0f)
      width = 1.0f;

    for (col = 0; col < num_cols; col ++)
    {
      col_widths[col] = col_mins[col] +
                        temp_width * (col_widths[col] - col_mins[col]) / width;

      DEBUG_printf(("    col_widths[%d] = %.1f\n", col, col_widths[col]));
    }

    for (col = 0, width = 0.0f; col < num_cols; col ++)
      width += col_widths[col] + 2 * cellpadding + cellspacing;

    width -= cellspacing;
  }

  DEBUG_puts("");

  DEBUG_printf(("Final table width = %.1f, alignment = %d\n",
                width, t->style->text_align));

  switch (t->style->text_align)
  {
    case HD_TEXTALIGN_LEFT :
        x = m.left() + cellpadding;
        break;
    case HD_TEXTALIGN_CENTER :
        x = m.left() + 0.5f * (m.width() - width) + cellpadding;
        break;
    case HD_TEXTALIGN_RIGHT :
        x = m.right() - width + cellpadding;
        break;
  }

  for (col = 0; col < num_cols; col ++)
  {
    col_lefts[col]  = x;
    col_rights[col] = x + col_widths[col];
    x = col_rights[col] + 2 * cellpadding + cellspacing;

    DEBUG_printf(("left[%d] = %.1f, right[%d] = %.1f\n", col, col_lefts[col], col,
                  col_rights[col]));
  }

  // Now render the whole table...
//  if (y < top && needspace)
//    y -= _htmlSpacings[SIZE_P];

  memset(row_spans, 0, sizeof(row_spans));
  memset(cell_start, 0, sizeof(cell_start));
  memset(cell_end, 0, sizeof(cell_end));
  memset(cell_height, 0, sizeof(cell_height));
  memset(cell_bg, 0, sizeof(cell_bg));

  for (row = 0; row < num_rows; row ++)
  {
    height_var = NULL;

    if (cells[row][0] != NULL)
    {
      // Do page comments...
      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->element == HD_ELEMENT_COMMENT)
        render_comment(cells[row][0]->parent->prev, m, x, y, page);

      // Get height...
      if ((height_var = t->get_attr("HEIGHT")) == NULL)
	if ((height_var = cells[row][0]->parent->get_attr("HEIGHT")) == NULL)
	  for (col = 0; col < num_cols; col ++)
	    if (cells[row][col]->get_attr("ROWSPAN") == NULL)
	      if ((height_var = cells[row][col]->get_attr("HEIGHT")) != NULL)
	        break;
    }

    if (cells[row][0] != NULL && height_var != NULL)
    {
      // Row height specified; make sure it'll fit...
      if (height_var[strlen(height_var) - 1] == '%')
	temp_height = atof(height_var) * 0.01f *
	              (css->media.page_print_length - 2 * cellpadding);
      else
        temp_height = atof(height_var) * 72.0f / css->ppi;

      if (t->get_attr("HEIGHT") != NULL)
        temp_height /= num_rows;

      temp_height -= 2 * cellpadding;
    }
    else
    {
      // Use min height computed from get_cell_size()...
      for (col = 0, temp_height = t->style->font_size * t->style->line_height;
           col < num_cols;
	   col ++)
        if (cells[row][col] != NULL &&
	    cells[row][col]->height > temp_height)
	  temp_height = cells[row][col]->height;

      if (temp_height > (media.page_length / 8) && height_var == NULL)
	temp_height = media.page_length / 8;
    }

    DEBUG_printf(("BEFORE row = %d, temp_height = %.1f, y = %.1f\n",
                  row, temp_height, y));

    if (y < (bottom + 2 * cellpadding + temp_height) &&
        temp_height <= (top - bottom - 2 * cellpadding))
    {
      DEBUG_puts("NEW PAGE");

      y = top;
      page ++;

      if (hdGlobal.verbosity)
        hdGlobal.progress_show("Formatting page %d", page);
    }

    do_valign  = 1;
    row_y      = y - cellpadding;
    row_page   = page;
    row_height = 0.0f;

    DEBUG_printf(("BEFORE row_y = %.1f, y = %.1f\n", row_y, y));

    for (col = 0, rowspan = 9999; col < num_cols; col += colspan)
    {
      if (row_spans[col] == 0)
      {
        if ((var = cells[row][col]->get_attr("ROWSPAN")) != NULL)
          row_spans[col] = atoi(var);

        if (row_spans[col] > (num_rows - row))
	  row_spans[col] = num_rows - row;

	span_heights[col] = 0.0f;
      }

      if (row_spans[col] < rowspan)
	rowspan = row_spans[col];

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
    }

    if (!rowspan)
      rowspan = 1;

    for (col = 0; col < num_cols;)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      DEBUG_printf(("    col = %d, colspan = %d, left = %.1f, right = %.1f, cell = %p\n",
                    col, colspan, col_lefts[col], col_rights[col + colspan], cells[row][col]));

      x        = col_lefts[col];
      temp_y    = y - cellpadding;
      temp_page = page;
      tempspace = 0;

      if (row == 0 || cells[row][col] != cells[row - 1][col])
      {
        check_pages(page);

        if (cells[row][col] == NULL)
	  bgcolor = NULL;
	else if ((bgcolor = cells[row][col]->get_attr("BGCOLOR")) == NULL)
          if ((bgcolor = cells[row][col]->parent->get_attr("BGCOLOR")) == NULL)
	    bgcolor = t->get_attr("BGCOLOR");

	if (bgcolor != NULL)
	{
	  memset(bgrgb, 0, sizeof(bgrgb));
          get_color(bgcolor, bgrgb);

	  width       = col_rights[col + colspan] - col_lefts[col] +
        	        2 * cellpadding;
	  border_left = col_lefts[col] - cellpadding;

          cell_bg[col] = add_render(page, HD_RENDERTYPE_BOX, border_left, row_y,
                                    width + border, 0.0, bgrgb);
	}
	else
	  cell_bg[col] = NULL;

	cell_start[col] = pages[page].last;
	cell_page[col]  = temp_page;
	cell_y[col]     = temp_y;

        if (cells[row][col] != NULL && cells[row][col]->child != NULL)
	{
	  DEBUG_printf(("    parsing cell %d,%d; width = %.1f\n", row, col,
	                col_rights[col + colspan] - col_lefts[col]));

          bottom += cellpadding;
	  top    -= cellpadding;

          cell_margin = new hdMargin(col_lefts[col], col_rights[col + colspan],
	                             bottom, top);

          render_doc(cells[row][col]->child, *cell_margin, x, temp_y, temp_page);

          delete cell_margin;

          bottom -= cellpadding;
	  top    += cellpadding;
        }

        cell_endpage[col] = temp_page;
        cell_endy[col]    = temp_y;
        cell_height[col]  = y - cellpadding - temp_y;
        cell_end[col]     = pages[page].last;

        if (cell_start[col] == NULL)
	  cell_start[col] = pages[page].first;

        DEBUG_printf(("row = %d, col = %d, y = %.1f, cell_y = %.1f, cell_height = %.1f\n",
	              row, col, y - cellpadding, temp_y, cell_height[col]));
      }

      if (row_spans[col] == 0 &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > row_height)
        row_height = cell_height[col];

      if (row_spans[col] < (rowspan + 1))
      {
	if (cell_page[col] != cell_endpage[col])
	  do_valign = 0;

        if (cell_endpage[col] > row_page)
	{
	  row_page = cell_endpage[col];
	  row_y    = cell_endy[col];
	}
	else if (cell_endy[col] < row_y && cell_endpage[col] == row_page)
	  row_y = cell_endy[col];
      }

      DEBUG_printf(("**** col = %d, row = %d, row_y = %.1f\n", col, row, row_y));

      for (col ++; colspan > 0; colspan --, col ++)
      {
        cell_start[col]   = NULL;
        cell_page[col]    = cell_page[col - 1];
        cell_y[col]       = cell_y[col - 1];
	cell_end[col]     = NULL;
        cell_endpage[col] = cell_endpage[col - 1];
        cell_endy[col]    = cell_endy[col - 1];
	cell_height[col]  = cell_height[col - 1];
      }
    }

    DEBUG_printf(("row = %d, row_y = %.1f, row_height = %.1f\n", row, row_y, row_height));

    for (col = 0; col < num_cols; col += colspan)
    {
      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;

      if (row_spans[col])
        span_heights[col] += row_height;

      DEBUG_printf(("col = %d, row_spans = %d, span_heights = %.1f, cell_height = %.1f\n",
                    col, row_spans[col], span_heights[col], cell_height[col]));

      if (row_spans[col] == rowspan &&
          cell_page[col] == cell_endpage[col] &&
	  cell_height[col] > span_heights[col])
      {
        temp_height = cell_height[col] - span_heights[col];
	row_height  += temp_height;
	DEBUG_printf(("Adjusting row-span height by %.1f, new row_height = %.1f\n",
	              temp_height, row_height));

	for (tcol = 0; tcol < num_cols; tcol ++)
	  if (row_spans[tcol])
	  {
	    span_heights[tcol] += temp_height;
	    DEBUG_printf(("col = %d, span_heights = %.1f\n", tcol, span_heights[tcol]));
	  }
      }
    }

    DEBUG_printf(("AFTER row = %d, row_y = %.1f, row_height = %.1f, y = %.1f, do_valign = %d\n",
                  row, row_y, row_height, y, do_valign));

    // Do the vertical alignment
    if (do_valign)
    {
      if (height_var != NULL)
      {
        // Hardcode the row height...
        if (height_var[strlen(height_var) - 1] == '%')
	  temp_height = atof(height_var) * 0.01f * css->media.page_print_length;
	else
          temp_height = atof(height_var) * 72.0f / css->ppi;

        if (t->get_attr("HEIGHT") != NULL)
          temp_height /= num_rows;

        if (temp_height > row_height)
	{
	  // Only enforce the height if it is > the actual row height.
	  row_height = temp_height;
          row_y      = y - temp_height;
	}
      }

      for (col = 0; col < num_cols; col += colspan + 1)
      {
        hdRenderNode	*p;
        float		delta_y;


        for (colspan = 1; (col + colspan) < num_cols; colspan ++)
          if (cells[row][col] != cells[row][col + colspan])
            break;

        colspan --;

        if (cell_start[col] == NULL || row_spans[col] > rowspan ||
	    cells[row][col] == NULL || cells[row][col]->child == NULL)
	  continue;

        if (row_spans[col])
          switch (cells[row][col]->style->vertical_align)
	  {
            case HD_VERTICALALIGN_MIDDLE :
        	delta_y = (span_heights[col] - cell_height[col]) * 0.5f;
        	break;

            case HD_VERTICALALIGN_BOTTOM :
        	delta_y = span_heights[col] - cell_height[col];
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }
	else
          switch (cells[row][col]->style->vertical_align)
	  {
            case HD_VERTICALALIGN_MIDDLE :
        	delta_y = (row_height - cell_height[col]) * 0.5f;
        	break;

            case HD_VERTICALALIGN_BOTTOM :
        	delta_y = row_height - cell_height[col];
        	break;

            default :
        	delta_y = 0.0f;
        	break;
          }

	DEBUG_printf(("row = %d, col = %d, valign = %d, cell_height = %.1f, span_heights = %.1f, delta_y = %.1f\n",
	              row, col, cells[row][col]->valignment,
		      cell_height[col], span_heights[col], delta_y));

        if (delta_y > 0.0f)
	{
	  if (cell_start[col] == cell_end[col])
	    p = cell_start[col];
	  else
	    p = cell_start[col]->next;

          for (; p != NULL; p = p->next)
	  {
	    DEBUG_printf(("aligning %p, y was %.1f, now %.1f\n",
	                  p, p->y, p->y - delta_y));

            p->y -= delta_y;
            if (p == cell_end[col])
	      break;
          }
        }
#ifdef DEBUG
        else
	{
	  if (cell_start[col] == cell_end[col])
	    p = cell_start[col];
	  else
	    p = cell_start[col]->next;

          for (; p != NULL; p = p->next)
	  {
	    printf("NOT aligning %p\n", p);

            if (p == cell_end[col])
	      break;
          }
	}
#endif // DEBUG
      }
    }

    // Update all current columns with ROWSPAN <= rowspan to use the same
    // end page...
    for (col = 1, temp_page = cell_endpage[0]; col < num_cols; col ++)
      if (cell_endpage[col] > temp_page && row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
        temp_page = cell_endpage[col];

    for (col = 0; col < num_cols; col ++)
      if (row_spans[col] <= rowspan &&
          cells[row][col] != NULL && cells[row][col]->child != NULL)
        cell_endpage[col] = temp_page;

    row_y -= cellpadding;

    for (col = 0; col < num_cols; col += colspan + 1)
    {
      if (row_spans[col] > 0)
      {
        DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
	              col, row_spans[col], row_spans[col] - rowspan));
        row_spans[col] -= rowspan;
      }

      for (colspan = 1; (col + colspan) < num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;

      width = col_rights[col + colspan] - col_lefts[col] +
              2 * cellpadding;

      if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
          row_spans[col] > 0)
        continue;

      if ((bgcolor = cells[row][col]->get_attr("BGCOLOR")) == NULL)
        if ((bgcolor = cells[row][col]->parent->get_attr("BGCOLOR")) == NULL)
	  bgcolor = t->get_attr("BGCOLOR");

      if (bgcolor != NULL)
        get_color(bgcolor, bgrgb, 0);

      border_left = col_lefts[col] - cellpadding;

      if (cell_page[col] != cell_endpage[col])
      {
        // Crossing a page boundary...
        if (border > 0)
	{
	  // +---+---+---+
	  // |   |   |   |

	  // Top
          add_render(cell_page[col], HD_RENDERTYPE_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          add_render(cell_page[col], HD_RENDERTYPE_BOX, border_left, bottom,
                     border, cell_y[col] - bottom + cellpadding + border, rgb);
	  // Right
          add_render(cell_page[col], HD_RENDERTYPE_BOX,
	             border_left + width, bottom,
		     border, cell_y[col] - bottom + cellpadding + border, rgb);
        }

        if (bgcolor != NULL)
        {
	  cell_bg[col]->y      = bottom;
	  cell_bg[col]->height = cell_y[col] - bottom + cellpadding + border;
	}

        for (temp_page = cell_page[col] + 1; temp_page != cell_endpage[col]; temp_page ++)
	{
	  // |   |   |   |
	  // |   |   |   |

	  if (border > 0.0f)
	  {
	    // Left
            add_render(temp_page, HD_RENDERTYPE_BOX, border_left, bottom,
                       border, top - bottom, rgb);
	    // Right
            add_render(temp_page, HD_RENDERTYPE_BOX,
	               border_left + width, bottom,
		       border, top - bottom, rgb);
          }

	  if (bgcolor != NULL)
            add_render(temp_page, HD_RENDERTYPE_BOX, border_left, bottom,
                       width + border, top - bottom, bgrgb, 1);
        }

        if (border > 0.0f)
	{
	  // |   |   |   |
	  // +---+---+---+

	  // Left
          add_render(cell_endpage[col], HD_RENDERTYPE_BOX, border_left, row_y,
                     border, top - row_y, rgb);
	  // Right
          add_render(cell_endpage[col], HD_RENDERTYPE_BOX,
	             border_left + width, row_y,
                     border, top - row_y, rgb);
	  // Bottom
          add_render(cell_endpage[col], HD_RENDERTYPE_BOX, border_left, row_y,
                     width + border, border, rgb);
        }

        if (bgcolor != NULL)
          add_render(cell_endpage[col], HD_RENDERTYPE_BOX, border_left, row_y,
	             width + border, top - row_y, bgrgb, 1);
      }
      else
      {
        // +---+---+---+
	// |   |   |   |
	// +---+---+---+
        if (border > 0.0f)
	{
	  // Top
          add_render(cell_page[col], HD_RENDERTYPE_BOX, border_left,
                     cell_y[col] + cellpadding,
		     width + border, border, rgb);
	  // Left
          add_render(cell_page[col], HD_RENDERTYPE_BOX, border_left, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Right
          add_render(cell_page[col], HD_RENDERTYPE_BOX,
	             border_left + width, row_y,
                     border, cell_y[col] - row_y + cellpadding + border, rgb);
	  // Bottom
          add_render(cell_page[col], HD_RENDERTYPE_BOX, border_left, row_y,
                     width + border, border, rgb);
	}

        if (bgcolor != NULL)
	{
	  cell_bg[col]->y      = row_y;
	  cell_bg[col]->height = cell_y[col] - row_y + cellpadding + border;
	}
      }
    }

    page = row_page;
    y    = row_y;

    if (row < (num_rows - 1))
      y -= cellspacing;

    DEBUG_printf(("END row = %d, y = %.1f, page = %d\n", row, y, page));
  }

  x = m.left();

  // Free memory for the table...
  if (num_rows > 0)
  {
    for (row = 0; row < num_rows; row ++)
      free(cells[row]);

    free(cells);
  }
}


//
// 'hdRender::get_cell_size()' - Get the size of a cell.
//

float						// O - Required width of cell
hdRender::get_cell_size(hdTree *t,		// I - Cell
        		float  left,		// I - Left margin
			float  right,		// I - Right margin
			float  *minwidth,	// O - Minimum width
			float  *prefwidth,	// O - Preferred width
			float  *minheight)	// O - Minimum height
{
  hdTree	*temp,				// Current tree entry
		*next;				// Next tree entry
  const char	*var;				// Attribute value
  int		nowrap;				// NOWRAP attribute?
  float		width,				// Width of cell
		frag_width,			// Fragment required width
		frag_height,			// Fragment height
		frag_pref,			// Fragment preferred width
		frag_min,			// Fragment minimum width
		minh,				// Local minimum height
		minw,				// Local minimum width
		prefw;				// Local preferred width


  DEBUG_printf(("get_cell_size(%p, %.1f, %.1f, %p, %p, %p)\n",
                t, left, right, minwidth, prefwidth, minheight));

  // First see if the width has been specified for this cell...
  if ((var = t->get_attr("WIDTH")) != NULL &&
      (var[strlen(var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen(var) - 1] == '%')
      width = (right - left) * atoi(var) * 0.01f;
    else
      width = atoi(var) * 72.0f / css->ppi;
  }
  else
    width = 0.0f;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = t->get_attr("HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen(var) - 1] == '%')
      minh = css->media.page_print_length * atoi(var) * 0.01f;
    else
      minh = atoi(var) * 72.0f / css->ppi;
  }
  else
    minh = 0.0f;

  nowrap = (t->get_attr("NOWRAP") != NULL);

  DEBUG_printf(("nowrap = %d\n", nowrap));

  for (temp = t->child, frag_width = 0.0f, frag_pref = 0.0f;
       temp != NULL;
       temp = next)
  {
    // Point to next markup, if any...
    next = temp->child;

    switch (temp->element)
    {
      case HD_ELEMENT_TABLE :
          // For nested tables, compute the width of the table.
          frag_width = get_table_size(temp, left, right, &frag_min,
	                              &frag_pref, &frag_height);

	  if (frag_pref > prefw)
	    prefw = frag_pref;

	  if (frag_min > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for nested table...\n",
	                  frag_min, minw));
	    minw = frag_min;
	  }

	  frag_width = 0.0f;
	  frag_pref  = 0.0f;
	  frag_min   = 0.0f;
	  next       = NULL;
	  break;

      case HD_ELEMENT_IMG :
          // Update the image width as needed...
	  if (temp->element == HD_ELEMENT_IMG)
	    update_size(temp);
      case HD_ELEMENT_NONE :
      case HD_ELEMENT_SPACER :
          frag_height = temp->height;

#ifdef TABLE_DEBUG
          if (temp->element == HD_ELEMENT_NONE)
	    printf("FRAG(%s) = %.1f\n", temp->data, temp->width);
	  else if (temp->element == HD_ELEMENT_SPACER)
	    printf("SPACER = %.1f\n", temp->width);
	  else
	    printf("IMG(%s) = %.1f\n", temp->get_attr("SRC"),
	           temp->width);
#endif // TABLE_DEBUG

          // Handle min/preferred widths separately...
          if (temp->width > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for fragment...\n",
	                  temp->width, minw));
	    minw = temp->width;
	  }

          if (temp->style->white_space == HD_WHITESPACE_PRE && temp->data != NULL &&
              temp->data[strlen(temp->data) - 1] == '\n')
          {
	    // End of a line - check preferred width...
	    frag_pref += temp->width + 1;

            if (frag_pref > prefw)
              prefw = frag_pref;

            if (temp->style->white_space == HD_WHITESPACE_PRE && frag_pref > minw)
	    {
	      DEBUG_printf(("Setting minw to %.1f (was %.1f) for preformatted...\n",
	                    frag_pref, minw));
              minw = frag_pref;
	    }

	    frag_pref = 0.0f;
          }
          else if (temp->data != NULL)
	    frag_pref += temp->width + 1;
	  else
	    frag_pref += temp->width;

          if ((temp->style->white_space == HD_WHITESPACE_PRE && temp->data != NULL &&
               temp->data[strlen(temp->data) - 1] == '\n') ||
	      (!temp->style->white_space == HD_WHITESPACE_PRE && temp->data != NULL &&
	       (isspace(temp->data[0]) ||
		isspace(temp->data[strlen(temp->data) - 1]))))
	  {
	    // Check required width...
            frag_width += temp->width + 1;

            if (frag_width > minw)
	    {
	      DEBUG_printf(("Setting minw to %.1f (was %.1f) for block...\n",
	                    frag_width, minw));
              minw = frag_width;
	    }

            frag_width = 0.0f;
	  }
	  else if (temp->data != NULL)
            frag_width += temp->width + 1;
	  else
	    frag_width += temp->width;
	  break;

      case HD_ELEMENT_ADDRESS :
      case HD_ELEMENT_BLOCKQUOTE :
      case HD_ELEMENT_BR :
      case HD_ELEMENT_CENTER :
      case HD_ELEMENT_DD :
      case HD_ELEMENT_DIV :
      case HD_ELEMENT_DT :
      case HD_ELEMENT_H1 :
      case HD_ELEMENT_H2 :
      case HD_ELEMENT_H3 :
      case HD_ELEMENT_H4 :
      case HD_ELEMENT_H5 :
      case HD_ELEMENT_H6 :
      case HD_ELEMENT_HR :
      case HD_ELEMENT_LI :
      case HD_ELEMENT_P :
      case HD_ELEMENT_PRE :
          DEBUG_printf(("BREAK at %.1f\n", frag_pref));

	  if (frag_pref > prefw)
	    prefw = frag_pref;

	  if (nowrap && frag_pref > minw)
	  {
	    DEBUG_printf(("Setting minw to %.1f (was %.1f) for break...\n",
	                  frag_pref, minw));
	    minw = frag_pref;
	  }

          frag_pref   = 0.0f;
	  frag_width  = 0.0f;
          frag_height = 0.0f;

      default :
	  break;
    }

    // Update minimum height...
    if (frag_height > minh)
      minh = frag_height;

    // Update next pointer as needed...
    if (next == NULL)
      next = temp->next;

    if (next == NULL)
    {
      // This code is almost funny if you say it fast... :)
      for (next = temp->parent; next != NULL && next != t; next = next->parent)
	if (next->next != NULL)
	  break;

      if (next == t)
	next = NULL;
      else if (next)
	next = next->next;
    }
  }

  // Check the last fragment's width...
  if (frag_pref > prefw)
    prefw = frag_pref;

  // Handle the "NOWRAP" option...
  if (nowrap && prefw > minw)
  {
    DEBUG_printf(("Setting minw to %.1f (was %.1f) for NOWRAP...\n",
	          prefw, minw));
    minw = prefw;
  }

  // Return the required, minimum, and preferred size of the cell...
  *minwidth  = minw;
  *prefwidth = prefw;
  *minheight = minh;

  DEBUG_printf(("get_cell_size(): width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
                width, minw, prefw, minh));

  return (width);
}


//
// 'hdRender::get_table_size()' - Get the size of a table.
//

float						// O - Minimum width of table
hdRender::get_table_size(hdTree *t,		// I - Table
                         float  left,		// I - Left margin
			 float  right,		// I - Right margin
		         float  *minwidth,	// O - Minimum width
			 float  *prefwidth,	// O - Preferred width
			 float  *minheight)	// O - Minimum height
{
  hdTree	*temp,			// Current tree entry
		*next;			// Next tree entry
  const char	*var;			// Attribute value
  float		width,			// Required width of table
		minw,			// Minimum width of table
		minh,			// Minimum height of table
		prefw,			// Preferred width of table
		cell_width,		// Cell required width
		cell_pref,		// Cell preferred width
		cell_min,		// Cell minimum width
		cell_height,		// Cell minimum height
		row_width,		// Row required width
		row_pref,		// Row preferred width
		row_min,		// Row minimum width
		row_height,		// Row minimum height
		border,			// Border around cells
		cellpadding,		// Padding inside cells
		cellspacing;		// Spacing around cells
  int		columns,		// Current number of columns
		max_columns,		// Maximum columns
		rows;			// Number of rows


  DEBUG_printf(("get_table_size(%p, %.1f, %.1f, %p, %p, %p)\n",
                t, left, right, minwidth, prefwidth, minheight));

  // First see if the width has been specified for this table...
  if ((var = t->get_attr("WIDTH")) != NULL &&
      (var[strlen(var) - 1] != '%' || (right - left) > 0.0f))
  {
    // Yes, use it!
    if (var[strlen(var) - 1] == '%')
      width = (right - left) * atoi(var) * 0.01f;
    else
      width = atoi(var) * 72.0f / css->ppi;
  }
  else
    width = 0.0f;

  minw  = 0.0f;
  prefw = 0.0f;

  // Then the height...
  if ((var = t->get_attr("HEIGHT")) != NULL)
  {
    // Yes, use it!
    if (var[strlen(var) - 1] == '%')
      minh = css->media.page_print_length * atoi(var) * 0.01f;
    else
      minh = atoi(var) * 72.0f / css->ppi;
  }
  else
    minh = 0.0f;

  // Update the size as needed...
  for (temp = t->child, row_width = 0.0f, row_min = 0.0f, row_pref = 0.0f,
	   row_height = 0.0f, columns = 0, rows = 0, max_columns = 0;
       temp != NULL;
       temp = next)
  {
    // Point to next markup, if any...
    next = temp->child;

    // Start a new row or add the cell width as needed...
    if (temp->element == HD_ELEMENT_TR)
    {
      minh += row_height;

      row_width  = 0.0f;
      row_pref   = 0.0f;
      row_min    = 0.0f;
      row_height = 0.0f;
      rows ++;
      columns = 0;
    }
    else if (temp->element == HD_ELEMENT_TD || temp->element == HD_ELEMENT_TH)
    {
      // Update columns...
      columns ++;
      if (columns > max_columns)
	max_columns = columns;

      // Get widths of cell...
      cell_width = get_cell_size(temp, left, right, &cell_min, &cell_pref,
                                 &cell_height);

      // Update row widths...
      row_width += cell_width;
      row_pref  += cell_pref;
      row_min   += cell_min;

      if (cell_height > row_height)
	row_height = cell_height;

      // Check current row widths against table...
      if (row_pref > prefw)
	prefw = row_pref;

      if (row_min > minw)
	minw = row_min;
    }

    // Update next pointer as needed...
    if (next == NULL)
      next = temp->next;

    if (next == NULL)
    {
      // This code is almost funny if you say it fast... :)
      for (next = temp->parent; next != NULL && next != t; next = next->parent)
	if (next->next != NULL)
	  break;

      if (next == t)
	next = NULL;
      else if (next)
	next = next->next;
    }
  }

  // Make sure last row is counted in min height calcs.
  minh += row_height;

  // Add room for spacing and padding...
  if ((var = t->get_attr("CELLPADDING")) != NULL)
    cellpadding = atoi(var);
  else
    cellpadding = 1.0f;

  if ((var = t->get_attr("CELLSPACING")) != NULL)
    cellspacing = atoi(var);
  else
    cellspacing = 0.0f;

  if ((var = t->get_attr("BORDER")) != NULL)
  {
    if ((border = atof(var)) == 0.0 && var[0] != '0')
      border = 1.0f;

    cellpadding += border;
  }
  else
    border = 0.0f;

  if (border == 0.0f && cellpadding > 0.0f)
  {
    // Ah, the strange table formatting nightmare that is HTML.
    // Netscape and MSIE assign an invisible border width of 1
    // pixel if no border is specified...
    cellpadding += 1.0f;
  }

  cellspacing *= 72.0f / css->ppi;
  cellpadding *= 72.0f / css->ppi;

  DEBUG_printf(("ADDING %.1f for table space...\n",
                max_columns * (2 * cellpadding + cellspacing) - cellspacing));

  if (width > 0.0f)
    width += max_columns * (2 * cellpadding + cellspacing) - cellspacing;

  minw  += max_columns * (2 * cellpadding + cellspacing) - cellspacing;
  prefw += max_columns * (2 * cellpadding + cellspacing) - cellspacing;
  minh  += rows * (2 * cellpadding + cellspacing) - cellspacing;

  // Return the required, minimum, and preferred size of the table...
  *minwidth  = minw;
  *prefwidth = prefw;
  *minheight = minh;

  DEBUG_printf(("get_table_size(): width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n",
                width, minw, prefw, minh));

  return (width);
}


//
// End of "$Id: render-table.cxx,v 1.7 2003/12/06 04:01:35 mike Exp $".
//
