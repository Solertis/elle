#include <elle/json/json.hh>

#include <elle/reactor/http/Request.hh>
#include <elle/reactor/http/exceptions.hh>
#include <elle/reactor/scheduler.hh>

int
main()
{
  reactor::Scheduler sched;
  reactor::Thread t(
    sched, "main",
    []
    {
      try
      {
        reactor::http::Request r("https://en.wikipedia.org/w/api.php",
                                 reactor::http::Method::GET);
        r.query_string({
            {"format", "json"},
            {"action", "query"},
            {"prop", "extracts"},
            {"explaintext", ""},
            {"exintro", ""},
            {"titles", "JSON"}
          });
        r.finalize();
        if (r.status() != reactor::http::StatusCode::OK)
          elle::err("quering %s failed", r);
        std::cout << elle::json::pretty_print(elle::json::read(r)) << std::endl;
      }
      catch (reactor::http::ResolutionFailure const& e)
      {
        std::cerr << "Unable to resolve " << e.url() << std::endl;
        throw;
      }
      catch (reactor::http::RequestError const& e)
      {
        std::cerr << "Unable to get " << e.url() << std::endl;
        throw;
      }
    });
  try
  {
    sched.run();
  }
  catch (...)
  {
    return 1;
  }
  return 0;
}
