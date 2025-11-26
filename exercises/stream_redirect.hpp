#pragma once

#include <ostream>
#include <streambuf>

/**
 * Adapted from
 *
 * https://stackoverflow.com/questions/77633473/how-to-redirect-stderr-and-stdcerr-to-a-stringstream
 */
class stream_redirect {
public:
  stream_redirect(std::ostream *old_stream, std::ostream *new_stream)
      : m_old_stream{old_stream}, m_old_streambuf{old_stream->rdbuf()},
        m_new_stream{new_stream} {
    old_stream->rdbuf(new_stream->rdbuf());
  }

  ~stream_redirect() { m_old_stream->rdbuf(m_old_streambuf); }

private:
  std::ostream *m_old_stream;
  std::streambuf *m_old_streambuf;
  std::ostream *m_new_stream;
};
